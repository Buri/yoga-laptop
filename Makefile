## -*- Makefile -*-
##
## User: buri
## Time: 2.5.2014 20:11:46
## Makefile created by Oracle Solaris Studio.
##
## This file is generated automatically.
##


## Target: all
driver:
	cd yoga_laptop && $(MAKE) install
sensors:
	cd sensors && $(MAKE) all
sensors-drivers:
	cd drivers && $(MAKE) default
all:
	$(MAKE) sensors sensors-drivers

#install:
#	cd yoga_laptop && $(MAKE) install
#	cd sensors/drivers && $(MAKE) install
#	cd sensors && $(MAKE) install

#### Clean target deletes all generated files ####
clean:

# Enable dependency checking
.KEEP_STATE:
.KEEP_STATE_FILE:.make.state.GNU-amd64-Linux

