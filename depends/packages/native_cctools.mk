package=native_cctools
$(package)_version=807d6fd1be5d2224872e381870c0a75387fe05e6
$(package)_download_path=https://github.com/theuni/cctools-port/archive
$(package)_file_name=$($(package)_version).tar.gz
$(package)_sha256_hash=a09c9ba4684670a0375e42d9d67e7f12c1f62581a27f28f7c825d6d7032ccc6a
$(package)_patches=fix-glibc-sysctl-header.patch
$(package)_build_subdir=cctools
$(package)_clang_version=8.0.0
$(package)_clang_download_path=http://llvm.org/releases/$($(package)_clang_version)
$(package)_clang_download_file=clang+llvm-$($(package)_clang_version)-x86_64-linux-gnu-ubuntu-18.04.tar.xz
$(package)_clang_file_name=clang-llvm-$($(package)_clang_version)-x86_64-linux-gnu-ubuntu-18.04.tar.xz
$(package)_clang_sha256_hash=0f5c314f375ebd5c35b8c1d5e5b161d9efaeff0523bac287f8b4e5b751272f51
$(package)_extra_sources=$($(package)_clang_file_name)

define $(package)_fetch_cmds
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_download_file),$($(package)_file_name),$($(package)_sha256_hash)) && \
$(call fetch_file,$(package),$($(package)_clang_download_path),$($(package)_clang_download_file),$($(package)_clang_file_name),$($(package)_clang_sha256_hash))
endef

define $(package)_extract_cmds
  mkdir -p $($(package)_extract_dir) && \
  echo "$($(package)_sha256_hash)  $($(package)_source)" > $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  echo "$($(package)_clang_sha256_hash)  $($(package)_source_dir)/$($(package)_clang_file_name)" >> $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  $(build_SHA256SUM) -c $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  mkdir -p toolchain/bin toolchain/lib/clang/3.5/include && \
  tar --strip-components=1 -C toolchain -xf $($(package)_source_dir)/$($(package)_clang_file_name) && \
  rm -f toolchain/lib/libc++abi.so* && \
  echo "#!/bin/sh" > toolchain/bin/$(host)-dsymutil && \
  echo "exit 0" >> toolchain/bin/$(host)-dsymutil && \
  chmod +x toolchain/bin/$(host)-dsymutil && \
  tar --strip-components=1 -xf $($(package)_source)
endef

define $(package)_set_vars
$(package)_config_opts=--target=$(host) --disable-lto-support
$(package)_ldflags+=-Wl,-rpath=\\$$$$$$$$\$$$$$$$$ORIGIN/../lib
$(package)_cc=$($(package)_extract_dir)/toolchain/bin/clang
$(package)_cxx=$($(package)_extract_dir)/toolchain/bin/clang++
endef

define $(package)_preprocess_cmds
  patch -p1 < $($(package)_patch_dir)/fix-glibc-sysctl-header.patch && \
  cd $($(package)_build_subdir); ./autogen.sh && \
  sed -i.old "/define HAVE_PTHREADS/d" ld64/src/ld/InputFiles.h
endef

define $(package)_config_cmds
  $($(package)_autoconf)
endef

define $(package)_build_cmds
  $(MAKE)
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install && \
  cd $($(package)_extract_dir)/toolchain && \
  mkdir -p $($(package)_staging_prefix_dir)/lib/clang/$($(package)_clang_version)/include && \
  mkdir -p $($(package)_staging_prefix_dir)/bin $($(package)_staging_prefix_dir)/include && \
  printf '%s\n' '#!/bin/bash' 'args=()' 'skip_next=0' 'for arg in "$$$$@"; do' '  if (( skip_next )); then skip_next=0; continue; fi' '  case "$$$$arg" in -fcoalesce-templates|-Wno-long-double) continue ;; -arch) skip_next=1; continue ;; esac' '  args+=("$$$$arg")' 'done' 'exec /usr/bin/clang "$$$${args[@]}"' > $($(package)_staging_prefix_dir)/bin/clang &&\
  printf '%s\n' '#!/bin/bash' 'args=()' 'skip_next=0' 'for arg in "$$$$@"; do' '  if (( skip_next )); then skip_next=0; continue; fi' '  case "$$$$arg" in -fcoalesce-templates|-Wno-long-double) continue ;; -arch) skip_next=1; continue ;; esac' '  args+=("$$$$arg")' 'done' 'exec /usr/bin/clang++ "$$$${args[@]}"' > $($(package)_staging_prefix_dir)/bin/clang++ &&\
  chmod +x $($(package)_staging_prefix_dir)/bin/clang $($(package)_staging_prefix_dir)/bin/clang++ &&\
  cp lib/libLTO.so $($(package)_staging_prefix_dir)/lib/ && \
  cp -rf lib/clang/$($(package)_clang_version)/include/* $($(package)_staging_prefix_dir)/lib/clang/$($(package)_clang_version)/include/ && \
  cp bin/dsymutil $($(package)_staging_prefix_dir)/bin/$(host)-dsymutil && \
  mv $($(package)_staging_prefix_dir)/bin/$(host)-ar $($(package)_staging_prefix_dir)/bin/$(host)-ar.cctools && \
  printf '%s\n' '#!/bin/bash' 'exec /usr/bin/llvm-ar "$$$$@"' > $($(package)_staging_prefix_dir)/bin/$(host)-ar && \
  chmod +x $($(package)_staging_prefix_dir)/bin/$(host)-ar && \
  mv $($(package)_staging_prefix_dir)/bin/$(host)-ranlib $($(package)_staging_prefix_dir)/bin/$(host)-ranlib.cctools && \
  printf '%s\n' '#!/bin/bash' 'exec /usr/bin/llvm-ranlib "$$$$@"' > $($(package)_staging_prefix_dir)/bin/$(host)-ranlib && \
  chmod +x $($(package)_staging_prefix_dir)/bin/$(host)-ranlib && \
  mv $($(package)_staging_prefix_dir)/bin/$(host)-nm $($(package)_staging_prefix_dir)/bin/$(host)-nm.cctools && \
  printf '%s\n' '#!/bin/bash' 'exec /usr/bin/llvm-nm "$$$$@"' > $($(package)_staging_prefix_dir)/bin/$(host)-nm && \
  chmod +x $($(package)_staging_prefix_dir)/bin/$(host)-nm && \
  mv $($(package)_staging_prefix_dir)/bin/$(host)-strip $($(package)_staging_prefix_dir)/bin/$(host)-strip.cctools && \
  printf '%s\n' '#!/bin/bash' 'exec /usr/bin/llvm-strip "$$$$@"' > $($(package)_staging_prefix_dir)/bin/$(host)-strip && \
  chmod +x $($(package)_staging_prefix_dir)/bin/$(host)-strip && \
  mv $($(package)_staging_prefix_dir)/bin/$(host)-ld $($(package)_staging_prefix_dir)/bin/$(host)-ld.cctools && \
  printf '%s\n' '#!/bin/bash' 'args=()' 'min_version=""' 'sdk_version=""' 'while [[ $$$$# -gt 0 ]]; do' '  case "$$$$1" in' '    -macosx_version_min)' '      min_version="$$$$2"; shift 2 ;;' '    -syslibroot)' '      args+=("$$$$1" "$$$$2")' '      sdk_path="$$$$2"' '      sdk_base="$$$${sdk_path##*/}"' '      sdk_version="$$$${sdk_base#MacOSX}"' '      sdk_version="$$$${sdk_version%.sdk}"' '      shift 2 ;;' '    *) args+=("$$$$1"); shift ;;' '  esac' 'done' 'if [[ -n "$$$$min_version" ]]; then' '  if [[ "$$$$min_version" == 1[2-9].* || "$$$$min_version" == [2-9][0-9].* ]]; then' '    min_version="11.0"' '  fi' '  [[ -z "$$$$sdk_version" ]] && sdk_version="$$$$min_version"' '  args+=("-platform_version" "macos" "$$$$min_version" "$$$$sdk_version")' 'fi' 'exec /usr/bin/lld -flavor darwin "$$$${args[@]}"' > $($(package)_staging_prefix_dir)/bin/$(host)-ld && \
  chmod +x $($(package)_staging_prefix_dir)/bin/$(host)-ld && \
  mv $($(package)_staging_prefix_dir)/bin/$(host)-libtool $($(package)_staging_prefix_dir)/bin/$(host)-libtool.cctools && \
  printf '%s\n' '#!/bin/bash' 'if [[ "$$$$1" == "-static" ]]; then' '  shift' 'fi' 'out=""' 'args=()' 'while [[ $$$$# -gt 0 ]]; do' '  case "$$$$1" in' '    -o) out="$$$$2"; shift 2 ;;' '    -*) echo "unsupported libtool option: $$$$1" >&2; exit 1 ;;' '    *) args+=("$$$$1"); shift ;;' '  esac' 'done' 'if [[ -z "$$$$out" ]]; then' '  echo "missing -o output" >&2' '  exit 1' 'fi' 'exec /usr/bin/llvm-ar rc "$$$$out" "$$$${args[@]}"' > $($(package)_staging_prefix_dir)/bin/$(host)-libtool && \
  chmod +x $($(package)_staging_prefix_dir)/bin/$(host)-libtool && \
  if `test -d include/c++/`; then cp -rf include/c++/ $($(package)_staging_prefix_dir)/include/; fi && \
  if `test -d lib/c++/`; then cp -rf lib/c++/ $($(package)_staging_prefix_dir)/lib/; fi
endef
