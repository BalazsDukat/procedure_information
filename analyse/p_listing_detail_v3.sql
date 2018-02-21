/* some 
testing text to
see if this works*/

# example description

## example category

CREATE DEFINER=`vt_proc_user`@`localhost` PROCEDURE p_listing_detail_v3
(
  IN p_in_listing_id INT UNSIGNED
,
  OUT p_out_json LONGBLOB
,
  OUT p_out_error SMALLINT UNSIGNED
, 
  OUT p_out_error_string VARCHAR(4096)
)
READS SQL DATA
listing_detail: BEGIN
  DECLARE v_listing_id INT DEFAULT 0;
  DECLARE v_block TEXT DEFAULT "p_listing_detail_v3";
  DECLARE v_row INT UNSIGNED DEFAULT 0; #-- this is just a counter
  DECLARE v_found_rows INT UNSIGNED DEFAULT 0;
  DECLARE v_customer_id INT UNSIGNED DEFAULT 0;
  DECLARE v_listing_customer_forename VARCHAR(255);
  DECLARE v_listing_customer_cp TINYINT(1);
  DECLARE v_listing_end_date DATETIME(6);
  DECLARE v_listed_card_model_id INT UNSIGNED DEFAULT NULL;
  DECLARE v_requested_card_model_id INT UNSIGNED DEFAULT NULL;
  DECLARE v_number_of_bids INT UNSIGNED DEFAULT 0;
  DECLARE v_number_of_card_models INT UNSIGNED DEFAULT 0;
  DECLARE v_direct TINYINT(1) UNSIGNED DEFAULT 0;
  DECLARE v_out_json LONGBLOB DEFAULT '';

  DECLARE listedCardModelIds CURSOR FOR
    SELECT 
        listing_card.card_model_id
      ,
        COUNT(1)
    FROM
      listing_card
    WHERE
      listing_card.listing_id = p_in_listing_id AND listing_card.tip = 'No'
    GROUP BY
      card_model_id
  ;
  
  DECLARE requestedCardModelIds CURSOR FOR
    SELECT 
        listing_card.card_model_id
    FROM
      listing_card
    WHERE
      listing_card.listing_id = p_in_listing_id AND listing_card.tip = 'Yes'
    GROUP BY
      card_model_id
  ;
  
  DECLARE EXIT HANDLER FOR SQLEXCEPTION
    CALL __p_error_handler( v_block, FALSE, NULL, NULL, p_out_error, p_out_error_string );

  SET p_out_error = 0, p_out_error_string = NULL;

  SELECT 
    listing.id
  ,
    IFNULL(customer_nickname.nickname, customer.forename) AS forename
  ,
    listing.customer_id
  ,
    IF(customer.account_type = 'certified_player', 1, 0) AS listing_cp
  , 
    listing.listing_end_date AS listing_end_date
  ,
    IF(listing.target_cust_id IS NOT NULL, 1, 0) AS direct
  FROM 
    listing 
  LEFT JOIN #-- listings without customer id are possible.
    customer
    ON customer.id = listing.customer_id
  LEFT JOIN
  customer_nickname
  ON 
  customer_nickname.customer_id = customer.id AND customer_nickname.active = 'Yes'
  WHERE 
    listing.id = p_in_listing_id  
  AND
    listing_active = 'Yes' 
  INTO 
    v_listing_id
  ,
    v_listing_customer_forename
  ,
    v_customer_id
  ,
    v_listing_customer_cp
  ,
    v_listing_end_date
  ,
    v_direct
  ;

  IF ROW_COUNT() = 0 THEN
    CALL __p_error_handler( v_block, FALSE, 45, CONCAT("Listing ", IFNULL(p_in_listing_id, "null"), " either does not exist or is no longer active"), p_out_error, p_out_error_string );
    LEAVE listing_detail;
  END IF;
  
  SELECT
    COUNT(1)
  FROM
    bid
  WHERE
    bid.listing_id = p_in_listing_id
  INTO
    v_number_of_bids
  ;
  
#---Dealing with wierd characters here in the future.

  SET p_out_json =
    CONCAT(
      '"listingId":'
    , 
      v_listing_id
    ,
      ',"nickname":"'
    ,
      _ej( IFNULL(v_listing_customer_forename, '') )
    ,
      '","custId":'
    ,
      IFNULL(v_customer_id, 0)
    ,
      ',"cp":'
    ,
      IF(v_listing_customer_cp = 1, 'true', 'false')
    ,
      ',"numberOfBids":'
    ,
      v_number_of_bids
    ,
      ',"endTime":'
    ,
      f_to_milli_epoch(v_listing_end_date)
    ,
      ',"direct":'
    ,
      IF(v_direct = 1, 'true', 'false')
    )
  ;
  
  OPEN listedCardModelIds;

  SET v_found_rows = FOUND_ROWS();
  
  IF v_found_rows > 0 THEN
  
    SET v_out_json = '';
  
    listing_loop: WHILE v_row < v_found_rows DO
  
      FETCH 
        listedCardModelIds 
      INTO 
          v_listed_card_model_id
        ,
          v_number_of_card_models
      ;
  
      SET v_out_json =
        CONCAT(
          v_out_json
        ,
          '{"cmid":'
        ,
          '"', v_listed_card_model_id, '"'
        ,
          ',"count":'
        ,
          v_number_of_card_models
        ,
          '}'
        )
      ;
      
      SET v_row = v_row + 1;
      
      IF v_row < v_found_rows THEN
        SET v_out_json = CONCAT(v_out_json, ",");
      END IF;
      
    END WHILE listing_loop;
    
    SET p_out_json =
      CONCAT(
          p_out_json
        ,
          ',"listedCardModelIds":['
        ,
          v_out_json
        ,
          ']'
        )
    ;
    
  END IF;
  
  CLOSE listedCardModelIds;
  
  OPEN requestedCardModelIds;

  SET v_found_rows = FOUND_ROWS();
  
  IF v_found_rows > 0 THEN
  
    SET v_row = 0, v_out_json = '';
    
    requesting_loop: WHILE v_row < v_found_rows DO

      FETCH 
        requestedCardModelIds 
      INTO 
          v_requested_card_model_id
      ;

      SET v_out_json =
        CONCAT(
          v_out_json
        ,
          v_requested_card_model_id
        )
      ;
      
      SET v_row = v_row + 1;
      
      IF v_row < v_found_rows THEN
        SET v_out_json = CONCAT(v_out_json, ",");
      END IF;
      
    END WHILE requesting_loop;
    
    SET p_out_json =
      CONCAT(
        p_out_json
      ,
        ',"requestedCardModelIds":['
      ,
        IFNULL(v_out_json, '')
      ,
        ']'
      )
    ;
    
  END IF;

  SET p_out_json =
    CONCAT(
        '{'
      ,
        IFNULL(p_out_json, '')
      ,
        '}'
      )
  ;
  
  IF p_out_json IS NULL THEN
    CALL __p_error_handler( v_block, FALSE, 247, "Output json is null", p_out_error, p_out_error_string );
    LEAVE listing_detail;
  END IF;  
  
END listing_detail