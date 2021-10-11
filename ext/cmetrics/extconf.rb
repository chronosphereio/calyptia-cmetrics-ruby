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

  def initialize(version=nil, **kwargs)
    @version = if version
                version
              else
                "master".freeze
              end
    @recipe = MiniPortileCMake.new("cmetrics", @version, **kwargs)
    @checkpoint = ".#{@recipe.name}-#{@recipe.version}.installed"
    @recipe.target = File.join(ROOT, "ports")
    @recipe.files << {
      url: "https://codeload.github.com/calyptia/cmetrics/tar.gz/v#{version}",
      sha256sum: "922965a3ccc5aeb9a423d9b82be06e7aaf37f2bc8736ad6e70ac1c8845155710",
    }
  end

  def build
    unless File.exist?(@checkpoint)
      @recipe.cook
      libcmetrics_path = Dir.glob(File.join(ROOT, "ports/#{@recipe.host}/cmetrics/#{@version}/lib*/libcmetrics.a")).first
      FileUtils.cp(libcmetrics_path, File.join(ROOT, "ext", "cmetrics", "libcmetrics.a"))
      libmpack_path = Dir.glob(File.join(ROOT, "ports/#{@recipe.host}/cmetrics/#{@version}/lib*/libmpack.a")).first
      FileUtils.cp(libmpack_path, File.join(ROOT, "ext", "cmetrics", "libmpack.a"))
      libxxhash_path = Dir.glob(File.join(ROOT, "ports/#{@recipe.host}/cmetrics/#{@version}/lib*/libxxhash.a")).first
      FileUtils.cp(libxxhash_path, File.join(ROOT, "ext", "cmetrics", "libxxhash.a"))
      include_path = File.join(ROOT, "ports/#{@recipe.host}/cmetrics/#{@version}/include/")
      FileUtils.cp_r(Dir.glob(File.join(include_path, "*")), File.join(ROOT, "ext", "cmetrics"))
      FileUtils.touch(@checkpoint)
    end
  end

  def activate
    @recipe.activate
  end
end

cmetrics = BuildCMetrics.new("0.2.2", cmake_command: determine_preferred_command("cmake3", "cmake"))
cmetrics.build

libdir = RbConfig::CONFIG["libdir"]
includedir = RbConfig::CONFIG["includedir"]

dir_config("cmetrics", includedir, libdir)
find_library("xxhash", nil, __dir__)
find_library("mpack", nil, __dir__)
find_library("cmetrics", nil, __dir__)

have_func("gmtime_s", "time.h")

create_makefile("cmetrics/cmetrics")
