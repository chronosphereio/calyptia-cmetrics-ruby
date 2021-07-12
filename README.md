# CMetrics Ruby

[![CI on Ubuntu](https://github.com/calyptia/cmetrics-ruby/actions/workflows/linux.yml/badge.svg?branch=main)](https://github.com/calyptia/cmetrics-ruby/actions/workflows/linux.yml)
[![CI on Windows](https://github.com/calyptia/cmetrics-ruby/actions/workflows/windows.yml/badge.svg?branch=main)](https://github.com/calyptia/cmetrics-ruby/actions/workflows/windows.yml)
[![CI on macOS](https://github.com/calyptia/cmetrics-ruby/actions/workflows/macos.yml/badge.svg?branch=main)](https://github.com/calyptia/cmetrics-ruby/actions/workflows/macos.yml)

A Ruby binding for [cmetrics](https://github.com/calyptia/cmetrics).

## Prerequisites

* Ruby 2.4 or later with its headers
* CMake 3.13 or later due to different directory TARGETS for bundled cmetrics installation
* gcc or clang or some equivalents toolchain

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

### Counter

`CMetrics::Counter` is a cumulative metric that can only increase monotonically and reset to zero at restart.
This class should be used for counting the total amount of record size or something like monotonic increasing values.

ref: [Metric Types -- Counter | Prometheus documentation](https://prometheus.io/docs/concepts/metric_types/#counter)

```ruby
require 'cmetrics'

@counter = CMetrics::Counter.new
@counter.create("kubernetes", "network", "load", "Network load", ["hostname", "app"])

@counter.val #=> nil
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

@gauge.val #=> nil
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

### Untyped

`CMetrics::Untyped` is a metric that can only set a value via `#set` and reset to zero at restart.
This class should be used for storing the increasing but discontinuous values.

```ruby
require 'cmetrics'

@untyped = CMetrics::Untyped.new
@untyped.create("kubernetes", "network", "load", "Network load", ["hostname", "app"])

@untyped.val #=> nil

@untyped.set 3.0 #=> true
@untyped.val #=> 3.0

# Multiple labels
@untyped.set(1.0, ["localhost", "cmetrics"]) #=> true
@untyped.val(["localhost", "cmetrics"]) #=> 1.0

@untyped.set(10.55, ["localhost", "test"]) #=> true
@untyped.val(["localhost", "test"]) #=> 10.55

#CMetrics::Untyped can set greater value than stored.
@untyped.set(12.15, ["localhost", "test"]) #=> true
#CMetrics::Untyped cannot set smaller value than stored.
@untyped.set(1, ["localhost", "test"]) #=> false
```

### Serde

`CMetrics::Serde` is for a decoding (and encoding to some format stuffs) from msgpacked buffers that are created by `CMetrics::Counter#to_msgpack` or `CMetrics::Gauge#to_msgpack`.

#### For Counter class instance(s)

```ruby
require 'cmetrics'

@counter = CMetrics::Counter.new
@counter.create("kubernetes", "network", "load", "Network load", ["hostname", "app"])
@counter.inc
@counter.inc(["calyptia.com", "cmetrics"])
@counter.inc(["calyptia.com", "cmetrics"])
@buffer = @counter.to_msgpack
@serde = CMetrics::Serde.new

@serde.from_msgpack(@buffer)
puts @serde #=> Decoded object is shown with text
```

#### For Gauge class instance(s)

```ruby
require 'cmetrics'

@gauge = CMetrics::Gauge.new
@gauge.create("kubernetes", "network", "load", "Network load", ["hostname", "app"])
@gauge.inc
@gauge.inc(["calyptia.com", "cmetrics"])
@gauge.inc(["calyptia.com", "cmetrics"])
@buffer = @gauge.to_msgpack
@serde = CMetrics::Serde.new

@serde.from_msgpack(@buffer)
puts @serde #=> Decoded object is shown with text
```

#### For wired buffer (multiple concatenated instances context)

```ruby
require 'cmetrics'

@gauge = CMetrics::Gauge.new
@gauge.create("kubernetes", "network", "load", "Network load", ["hostname", "app"])
@gauge.set 2.0
@gauge.inc(["localhost", "cmetrics"])
@gauge.add(10, ["localhost", "test"])
@counter = CMetrics::Counter.new
@counter.create("kubernetes", "network", "load", "Network load", ["hostname", "app"])
@counter.inc
@counter.inc(["localhost", "cmetrics"])
@counter.add(10.55, ["localhost", "test"])
@counter2 = CMetrics::Counter.new
@counter2.create("cmt", "labels", "test", "Static labels test", ["host", "app"])
@counter2.inc
@counter2.inc(["calyptia.com", "cmetrics"])
@counter2.inc(["calyptia.com", "cmetrics"])
@counter2.add_label("dev", "Calyptia")
@counter2.add_label("lang", "C")
@wired_buffer = @gauge.to_msgpack + @counter.to_msgpack + @counter2.to_msgpack
@serde = CMetrics::Serde.new

puts "-----Decode with procedural style-----"
# Decode for the context 1
@serde.from_msgpack(@wired_buffer)
puts @serde.to_prometheus
# Decode for the context 2
@serde.from_msgpack(@wired_buffer)
puts @serde.to_prometheus
# Decode for the context 3
@serde.from_msgpack(@wired_buffer)
puts @serde.to_prometheus

puts "-----Decode with streaming style-----"
# Or, use streaming style API
@serde.feed_each(@wired_buffer) do |serde|
  puts serde.to_prometheus
end
```

## Development

After checking out the repo, run `bin/setup` to install dependencies. Then, run `rake test-unit` to run the tests. You can also run `bin/console` for an interactive prompt that will allow you to experiment.

To install this gem onto your local machine, run `bundle exec rake install`. To release a new version, update the version number in `version.rb`, and then run `bundle exec rake release`, which will create a git tag for the version, push git commits and the created tag, and push the `.gem` file to [rubygems.org](https://rubygems.org).

## Contributing

Bug reports and pull requests are welcome on GitHub at https://github.com/calyptia/cmetrics-ruby.
