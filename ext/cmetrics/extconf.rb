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

class BuildCMetrics

  attr_reader :recipe

  def initialize(version=nil)
    @version = if version
                version
              else
                "master".freeze
              end
    @recipe = MiniPortileCMake.new("cmetrics", @version)
    @checkpoint = ".#{@recipe.name}-#{@recipe.version}.installed"
    @recipe.target = File.join(ROOT, "ports")
    @recipe.files << {
      url: "file://#{ROOT}/ext/#{@recipe.name}-#{@recipe.version}.tar.gz",
      sha256sum: "34df0842ee23e4c9440e787722fb79cf7204ebb3fed2dc3fb5e02f4c9c4807dc",
    }
  end

  def build
    unless File.exist?(@checkpoint)
      @recipe.cook
      libcmetrics_path = File.join(ROOT, "ports/#{@recipe.host}/cmetrics/#{@version}/lib/libcmetrics.a")
      FileUtils.cp(libcmetrics_path, File.join(ROOT, "ext", "cmetrics", "libcmetrics.a"))
      libmpack_path = File.join(ROOT, "ports/#{@recipe.host}/cmetrics/#{@version}/lib/libmpack.a")
      FileUtils.cp(libmpack_path, File.join(ROOT, "ext", "cmetrics", "libmpack.a"))
      libxxhash_path = File.join(ROOT, "ports/#{@recipe.host}/cmetrics/#{@version}/lib/libxxhash.a")
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

cmetrics = BuildCMetrics.new
cmetrics.build

libdir = RbConfig::CONFIG["libdir"]
includedir = RbConfig::CONFIG["includedir"]

dir_config("cmetrics", includedir, libdir)
find_library("xxhash", nil, __dir__)
find_library("mpack", nil, __dir__)
find_library("cmetrics", nil, __dir__)

have_func("gmtime_s", "time.h")

create_makefile("cmetrics/cmetrics")
