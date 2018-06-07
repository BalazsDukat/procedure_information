# procedure_information
Auto-generates documentation for MySQL stored procedures' source files.
Originally made for (and used by) for a previus emplyer, VirtTrade Ltd. 
for generating basic documentation for the .sql sourcode files of about 200+ procedures
written for the company's MariaDB system. That documentation was then 
used by the back-end developers of the company.
Originally I used this on Windows 10 64 bit, but can be recompiled for any other operating system
that has a C++ compiler available for it as this software does not rely on anything special
apar from the Standard Template Library (STL) that should be available.

Instruction for setup:

1. Compiled the sourcecode in the src folder.
2. Copy the resuing executable binary file in the root folder ("procedure_information") of this software.
Or make sure there is an "analyse" and an "md" folder available next to the executabe.

Instuctions of use:

1. Copy the .sql files into the "analyse" folder.
2. From command line, start the software. (No parameters required.)
3. Go into the "md" folder. The generated documentation will be there;
one file for every single .sql file, in md format (so with .md extension)
which can be opened in browsers such as Mozilla Firefox, using an extension of some sort.
(In the future  might modify this to generate .html instead, but .md was the expected format at the time.)
