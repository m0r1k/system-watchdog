.PHONY: deb check create deploy-deb

SHELL := /bin/bash

## should be without quotes
REPLACE_IN_FILES=control postinst prerm postrm

ifeq ($(strip $(TARGET)),)
    $(error TARGET is not set)
endif

ifeq ($(strip $(PACKAGE_NAME)),)
    $(error PACKAGE_NAME is not set)
endif

ifeq ($(strip $(DEBUG)),)
    PACKAGE_NAME_FULL = $(PACKAGE_NAME)
else
    PACKAGE_NAME_FULL = $(PACKAGE_NAME)-debug
endif

FAKEROOT = /tmp/$(PACKAGE_NAME_FULL)

$(info TARGET=$(TARGET))
$(info GIT_BRANCH=$(GIT_BRANCH))
$(info PACKAGE_NAME=$(PACKAGE_NAME))
$(info PACKAGE_NAME_FULL=$(PACKAGE_NAME_FULL))
$(info FAKEROOT=$(FAKEROOT))
$(info DEBUG=$(DEBUG))
$(info REPONAME=$(REPONAME))

deb:
	@if [ ! -n "$(REPONAME)" ];then                         \
        echo "REPONAME env var is not set";                 \
        echo "usage example: make deb REPONAME=bullseye";   \
        exit 1;                                             \
    fi

	rm -rf $(FAKEROOT)

    # create project folders
	install -d $(FAKEROOT)
	install -d $(FAKEROOT)/DEBIAN/
	install -d $(FAKEROOT)/usr/share/doc/$(PACKAGE_NAME_FULL)
	install -d $(FAKEROOT)/usr/src/$(PACKAGE_NAME)

    # copy configs
	cp -r rootfs/* $(FAKEROOT)/

    # copy project files
	cp -r src $(FAKEROOT)/usr/src/$(PACKAGE_NAME)

    # add project controlfiles
	cp deploy/$(REPONAME)/DEBIAN/* $(FAKEROOT)/DEBIAN/
	cp deploy/$(REPONAME)/DEBIAN/copyright \
        $(FAKEROOT)/usr/share/doc/$(PACKAGE_NAME_FULL)/
	rm -f $(FAKEROOT)/DEBIAN/copyright
	gzip -c -9 deploy/$(REPONAME)/DEBIAN/changelog \
        > $(FAKEROOT)/usr/share/doc/$(PACKAGE_NAME_FULL)/changelog.Debian.gz

	for i in $(REPLACE_IN_FILES);do                                        \
        cur_fpath=$(FAKEROOT)/DEBIAN/$$i;                                  \
        sed -i 's/#package_section/$(PACKAGE_SECTION)/'       $$cur_fpath; \
        sed -i 's/#package_priority/$(PACKAGE_PRIORITY)/'     $$cur_fpath; \
        sed -i 's/#package_name_full/$(PACKAGE_NAME_FULL)/'   $$cur_fpath; \
        sed -i 's/#package_name/$(PACKAGE_NAME)/'             $$cur_fpath; \
        sed -i 's/#version/$(PACKAGE_VERSION)/'               $$cur_fpath; \
        sed -i 's/#build/$(BUILD_NUMBER)/'                    $$cur_fpath; \
        sed -i 's/#package_descr/$(PACKAGE_DESCR)/'           $$cur_fpath; \
        sed -i 's/#package_maintainer/$(PACKAGE_MAINTAINER)/' $$cur_fpath; \
        sed -i 's/#package_arch/$(PACKAGE_ARCH)/'             $$cur_fpath; \
        sed -i 's/#package_depends/$(PACKAGE_DEPENDS)/'       $$cur_fpath; \
    done

    # fix rights and build deb
	sudo chmod 755 $(FAKEROOT)/DEBIAN/*
	sudo chmod 644 $(FAKEROOT)/DEBIAN/conffiles
	fakeroot dpkg-deb --build $(FAKEROOT)

	cp /tmp/$(PACKAGE_NAME_FULL).deb \
        ./$(PACKAGE_NAME_FULL)_$(PACKAGE_VERSION)+${BUILD_NUMBER}.deb

check: deb
	lintian --allow-root \
        -L ">=important" \
            $(PACKAGE_NAME_FULL)_$(PACKAGE_VERSION)+${BUILD_NUMBER}.deb \
                || true

create: check
	sudo mv ./$(PACKAGE_NAME_FULL)_$(PACKAGE_VERSION)+${BUILD_NUMBER}.deb \
        ./$(PACKAGE_NAME_FULL)_$(PACKAGE_VERSION)+${BUILD_NUMBER}~$(REPONAME).deb

deploy-deb:
	sudo install -d -m 777 /usr/local/$(REPONAME)/archive/$(PACKAGE_NAME_FULL)
	sudo cp ./$(PACKAGE_NAME_FULL)_*~$(REPONAME).deb \
        /usr/local/$(REPONAME)/archive/$(PACKAGE_NAME_FULL)/
	sudo cp -f ./$(PACKAGE_NAME_FULL)_$(PACKAGE_VERSION)+${BUILD_NUMBER}~$(REPONAME).deb \
        /usr/local/$(REPONAME)/archive/$(PACKAGE_NAME_FULL)/$(PACKAGE_NAME_FULL).deb

create-debug:
	$(MAKE) create              \
        REPONAME=$(REPONAME)    \
        DEBUG=1

