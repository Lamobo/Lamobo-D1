add_library( sqlite STATIC sqlite3.c sqlite3.h )
set_property( TARGET sqlite PROPERTY FOLDER "ThirdParty")
