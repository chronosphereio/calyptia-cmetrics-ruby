# -*- encoding: utf-8 -*-
$LOAD_PATH.push File.expand_path('../lib', __FILE__)
require 'cmetrics/version'

Gem::Specification.new do |spec|
  spec.name          = "cmetrics"
  spec.version       = CMetrics::VERSION
  spec.authors       = ["Hiroshi Hatake"]
  spec.email         = ["cosmo0920.oucc@gmail.com"]

  spec.summary       = "C binding for cmetric library."
  spec.description   = spec.summary
  spec.homepage      = "https://github.com/calyptia/cmetrics-ruby"
  spec.required_ruby_version = Gem::Requirement.new(">= 2.4.0")

  spec.metadata["homepage_uri"] = spec.homepage
  spec.metadata["changelog_uri"] = "https://github.com/calyptia/cmetrics-ruby/blob/master/CHANGELOG.md"
  spec.extensions = ["ext/cmetrics/extconf.rb"]
  spec.license = "Apache-2.0"

  # Specify which files should be added to the gem when it is released.
  # The `git ls-files -z` loads the files in the RubyGem that have been added into git.
  spec.files = `git ls-files -z`.split("\x0").reject { |f| f.match(%r{\A(?:test|spec|features)/}) }
  spec.bindir        = "exe"
  spec.executables   = spec.files.grep(%r{\Aexe/}) { |f| File.basename(f) }
  spec.require_paths = ["lib"]

  spec.add_development_dependency "bundler"
  spec.add_development_dependency "rake", ">= 0"
  spec.add_development_dependency "rake-compiler", "~> 1.0"
  spec.add_development_dependency "rake-compiler-dock", "~> 1.0"

  spec.add_dependency 'mini_portile2', '~> 2.7'
end
