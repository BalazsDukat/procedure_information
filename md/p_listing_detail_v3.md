####  example category

####  example description

##### **p_listing_detail_v3**(*p_in_listing_id, p_out_json, p_out_error, p_out_error_string*)

#### Depends on:
- [__p_error_handler](__p_error_handler.md)


Parameter | In/Out | Type | Description
--- |--- |--- |---
```p_in_listing_id```|IN|INT|
```p_out_json```|OUT|LONGBLOB|
```p_out_error```|OUT|SMALLINT|
```p_out_error_string```|OUT|VARCHAR(4096)|


~~~sql
CALL p_listing_detail_v3(p_in_listing_id, p_out_json, p_out_error, p_out_error_string)
~~~
 some 
testing text to
see if this works

#### Reads from:
- listing_card
- listing
- customer
- customer_nickname
- bid


#### Error codes that can be thrown by this:
45, 247