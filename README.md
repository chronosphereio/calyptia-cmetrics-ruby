# CMetrics Ruby

[![CI on Ubuntu](https://github.com/calyptia/cmetrics-ruby/actions/workflows/linux.yml/badge.svg?branch=main)](https://github.com/calyptia/cmetrics-ruby/actions/workflows/linux.yml)
[![CI on Windows](https://github.com/calyptia/cmetrics-ruby/actions/workflows/windows.yml/badge.svg?branch=main)](https://github.com/calyptia/cmetrics-ruby/actions/workflows/windows.yml)
[![CI on macOS](https://github.com/calyptia/cmetrics-ruby/actions/workflows/macos.yml/badge.svg?branch=main)](https://github.com/calyptia/cmetrics-ruby/actions/workflows/macos.yml)

A Ruby binding for [cmetrics](https://github.com/calyptia/cmetrics).

## Installation

Add this line to your application's Gemfile:

```ruby
gem 'cmetrics'
```

And then execute:

    $ bundle install

Or install it yourself as:

    $ gem install cmetrics

## Usage

`CMetrics::Counter` is a cumulative metric that can only increase monotonically and reset to zero at restart.
This class should be used for counting the total amount of record size or something like monotonic increasing values.

ref: [Metric Types -- Counter | Prometheus documentation](https://prometheus.io/docs/concepts/metric_types/#counter)

### Counter

```ruby
require 'cmetrics'

@counter = CMetrics::Counter.new
@counter.create("kubernetes", "network", "load", "Network load", ["hostname", "app"])

@counter.val #=> 0.0
@counter.inc #=> true

@counter.val #=> 1.0
@counter.add 2.0 #=> true
@counter.val #=> 3.0

# Multiple labels
@counter.inc(["localhost", "cmetrics"]) #=> true
@counter.val(["localhost", "cmetrics"]) #=> 1.0

@counter.add(10.55, ["localhost", "test"]) #=> true
@counter.val(["localhost", "test"]) #=> 10.55

#CMetrics::Counter can set greater value than stored.
@counter.set(12.15, ["localhost", "test"]) #=> true
#CMetrics::Counter cannot set smaller value than stored.
@counter.set(1, ["localhost", "test"]) #=> false
```

### Gauge

`CMetrics::Gauge` is a metric that can represent arbitrary values and arbitrarily go up and down.
This class should be used for counting to be going up and down values such as CPU and memory usages, queued buffer size or something can go up, down and to be zero.

ref: [Metric Types -- Gauge | Prometheus documentation](https://prometheus.io/docs/concepts/metric_types/#gauge)

```ruby
require 'cmetrics'

@gauge = CMetrics::Gauge.new
@gauge.create("kubernetes", "network", "load", "Network load", ["hostname", "app"])

@gauge.val #=> 0.0
@gauge.inc #=> true

@gauge.val #=> 1.0
@gauge.add 2.0 #=> true
@gauge.val #=> 3.0

# Multiple labels
@gauge.inc(["localhost", "cmetrics"]) #=> true
@gauge.val(["localhost", "cmetrics"]) #=> 1.0

@gauge.add(10, ["localhost", "test"]) #=> true
@gauge.val(["localhost", "test"]) #=> 10

@gauge.sub(2.5, ["localhost", "test"]) #=> true
@gauge.val(["localhost", "test"]) #=> 7.5
```

## Development

After checking out the repo, run `bin/setup` to install dependencies. Then, run `rake test-unit` to run the tests. You can also run `bin/console` for an interactive prompt that will allow you to experiment.

To install this gem onto your local machine, run `bundle exec rake install`. To release a new version, update the version number in `version.rb`, and then run `bundle exec rake release`, which will create a git tag for the version, push git commits and the created tag, and push the `.gem` file to [rubygems.org](https://rubygems.org).

## Contributing

Bug reports and pull requests are welcome on GitHub at https://github.com/calyptia/cmetrics-ruby.
