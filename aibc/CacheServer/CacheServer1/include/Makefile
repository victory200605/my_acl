include ../common.mk

Config = "const size_t AI_MAX_KEY_LEN = $(CACHESERVER_KEY_LEN);const size_t AI_MAX_VALUE_LEN = $(CACHESERVER_VALUE_LEN);"

all:
	@echo $(Config) > v1/cacheserver/CacheConfig.inc
	@echo $(Config) > v2/cacheserver/CacheConfig.inc
	@echo $(Config) > v3/cacheserver/CacheConfig.inc

clean:
	@$(RM) v1/cacheserver/CacheConfig.inc
	@$(RM) v2/cacheserver/CacheConfig.inc
