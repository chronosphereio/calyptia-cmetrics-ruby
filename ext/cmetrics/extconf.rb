require "mkmf"
require "rbconfig"
require "mini_portile2"
require "fileutils"

ROOT = File.expand_path(File.join(File.dirname(__FILE__), '..', '..'))

def linux?
  RUBY_PLATFORM =~ /linux/
end

def windows?
  RUBY_PLATFORM =~ /mingw|mswin/
end

def determine_preferred_command(bin, default_bin)
  printf "checking for whether %s or %s is usable... ", bin, default_bin
  STDOUT.flush
  bin += RbConfig::CONFIG['EXEEXT']
  path = ENV['PATH'].split(RbConfig::CONFIG['PATH_SEPARATOR'])
  for dir in path
    file = File.join(dir, bin)
    if FileTest.executable?(file)
      printf "%s\n", bin
      return bin
    else
      next
    end
  end
  printf "%s\n", default_bin
  return default_bin
end

class BuildCMetrics

  attr_reader :recipe

  def initialize(version=nil, fluent_otel_version= "0.9.0", cfl_version= "0.2.0", **kwargs)
    @version = if version
                version
              else
                "master".freeze
              end
    @fluent_otel_version = fluent_otel_version
    @cfl_version = cfl_version
    @recipe = MiniPortileCMake.new("cmetrics", @version, **kwargs)
    @checkpoint = ".#{@recipe.name}-#{@recipe.version}.installed"
    @recipe.target = File.join(ROOT, "ports")
    @recipe.files << {
      url: "https://codeload.github.com/fluent/cmetrics/tar.gz/v#{version}",
      sha256sum: "e576e2cb6a7784a8094b0a050d918d48ca913740d4c1bfee3d8645a7bfe8b2bd",
    }

    @otel_proto_recipe = MiniPortileCMake.new("fluent-otel-proto", @fluent_otel_version, **kwargs)
    @otel_proto_checkpoint = ".#{@otel_proto_recipe.name}-#{@otel_proto_recipe.version}.installed"
    @otel_proto_recipe.target = File.join(ROOT, "ports")
    @otel_proto_recipe.files << {
      url: "https://codeload.github.com/fluent/fluent-otel-proto/tar.gz/v#{fluent_otel_version}",
      sha256sum: "0a31b14050ceabbee74a76b6b92d5c11532e99cea1e8f7b29806bea559379b05",
    }

    @cfl_recipe = MiniPortileCMake.new("cfl", @cfl_version, **kwargs)
    @cfl_checkpoint = ".#{@cfl_recipe.name}-#{@cfl_recipe.version}.installed"
    @cfl_recipe.target = File.join(ROOT, "ports")
    @cfl_recipe.files << {
      url: "https://codeload.github.com/fluent/cfl/tar.gz/v#{cfl_version}",
      sha256sum: "61caf08a2e428d660304ab86061e61a09577e9ecc140b1a0f387d8bc39b41b9d",
    }
  end

  def tmp_cmetrics_root_path
    "tmp/#{@recipe.host}/ports/#{@recipe.name}/#{@recipe.version}"
  end

  def tmp_cmetrics_path
    File.join(tmp_cmetrics_root_path, "#{@recipe.name}-#{@recipe.version}")
  end

  def tmp_cfl_root_path
    "tmp/#{@cfl_recipe.host}/ports/#{@cfl_recipe.name}/#{@cfl_recipe.version}"
  end

  def tmp_cfl_path
    File.join(tmp_cfl_root_path, "#{@cfl_recipe.name}-#{@cfl_recipe.version}")
  end

  def tmp_otel_proto_root_path
    "tmp/#{@otel_proto_recipe.host}/ports/#{@otel_proto_recipe.name}/#{@otel_proto_recipe.version}"
  end

  def tmp_otel_proto_path
    File.join(tmp_otel_proto_root_path, "#{@otel_proto_recipe.name}-#{@otel_proto_recipe.version}")
  end

  def prepare_cfl
    unless File.exist?(@cfl_checkpoint)
      @cfl_recipe.download
      @cfl_recipe.extract
      FileUtils.cp_r(Dir.glob(File.join(tmp_cfl_path, "*")), File.join(tmp_cmetrics_path, "lib", @cfl_recipe.name))
      # FileUtils.touch(@cfl_checkpoint)
    end
  end

  def prepare_otel_proto
    unless File.exist?(@otel_proto_checkpoint)
      @otel_proto_recipe.download
      @otel_proto_recipe.extract
      FileUtils.cp_r(Dir.glob(File.join(tmp_otel_proto_path, "*")), File.join(tmp_cmetrics_path, "lib", @otel_proto_recipe.name))
      # FileUtils.touch(@otel_proto_checkpoint)
    end
  end

  def build
    unless File.exist?(@checkpoint)
      @recipe.download
      @recipe.extract
      prepare_cfl
      prepare_otel_proto
      @recipe.patch
      @recipe.configure
      @recipe.compile
      @recipe.install
      libcmetrics_path = Dir.glob(File.join(ROOT, "ports/#{@recipe.host}/cmetrics/#{@version}/lib*/libcmetrics.a")).first
      FileUtils.cp(libcmetrics_path, File.join(ROOT, "ext", "cmetrics", "libcmetrics.a"))
      libmpack_path = Dir.glob(File.join(ROOT, "ports/#{@recipe.host}/cmetrics/#{@version}/lib*/libmpack.a")).first
      FileUtils.cp(libmpack_path, File.join(ROOT, "ext", "cmetrics", "libmpack.a"))
      libxxhash_path = Dir.glob(File.join(ROOT, "ports/#{@recipe.host}/cmetrics/#{@version}/lib*/libxxhash.a")).first
      FileUtils.cp(libxxhash_path, File.join(ROOT, "ext", "cmetrics", "libxxhash.a"))
      libcfl_path = Dir.glob(File.join(ROOT, "ports/#{@recipe.host}/cmetrics/#{@version}/lib*/libcfl.a")).first
      FileUtils.cp(libcfl_path, File.join(ROOT, "ext", "cmetrics", "libcfl.a"))
      libotel_proto_path = Dir.glob(File.join(ROOT, "ports/#{@recipe.host}/cmetrics/#{@version}/lib*/libfluent-otel-proto.a")).first
      FileUtils.cp(libotel_proto_path, File.join(ROOT, "ext", "cmetrics", "libfluent-otel-proto.a"))
      include_path = File.join(ROOT, "ports/#{@recipe.host}/cmetrics/#{@version}/include/")
      FileUtils.cp_r(Dir.glob(File.join(include_path, "*")), File.join(ROOT, "ext", "cmetrics"))
      FileUtils.touch(@checkpoint)
    end
  end

  def activate
    @recipe.activate
  end
end

cmetrics = BuildCMetrics.new("0.5.8", cmake_command: determine_preferred_command("cmake3", "cmake"))
cmetrics.build

libdir = RbConfig::CONFIG["libdir"]
includedir = RbConfig::CONFIG["includedir"]

dir_config("cmetrics", includedir, libdir)
find_library("xxhash", nil, __dir__)
find_library("cfl", nil, __dir__)
find_library("mpack", nil, __dir__)
find_library("cmetrics", nil, __dir__)
find_library("fluent-otel-proto", nil, __dir__)

have_func("gmtime_s", "time.h")

create_makefile("cmetrics/cmetrics")
