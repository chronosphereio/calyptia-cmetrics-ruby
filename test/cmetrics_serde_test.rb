require "test_helper"
require "msgpack"

class CMetricsSerdeTest < Test::Unit::TestCase
  sub_test_case "Serde" do
    setup do
      @serde = CMetrics::Serde.new
    end

    sub_test_case "uninitialized cmt contexts" do
      setup do
        @serde = CMetrics::Serde.new
      end

      test "to_prometheus" do
        assert_raise(RuntimeError) do
          @serde.to_prometheus
        end
      end

      test "to_msgpack" do
        assert_raise(RuntimeError) do
          @serde.to_msgpack
        end
      end

      test "to_influx" do
        assert_raise(RuntimeError) do
          @serde.to_influx
        end
      end

      test "to_s" do
        assert_raise(RuntimeError) do
          @serde.to_s
        end
      end

      test "prometheus_remote_write" do
        assert_raise(RuntimeError) do
          @serde.prometheus_remote_write
        end
      end
    end

    sub_test_case "w/ counter" do
      setup do
        @counter = CMetrics::Counter.new
        @counter.create("kubernetes", "network", "load", "Network load", ["hostname", "app"])
        @counter.inc
        @counter.inc(["calyptia.com", "cmetrics"])
        @counter.inc(["calyptia.com", "cmetrics"])
        @buffer = @counter.to_msgpack
      end

      test "decode counter" do
        assert_true @serde.from_msgpack(@buffer)
        buffer = @serde.to_msgpack
        assert_equal buffer.size, @buffer.size
      end

      test "prometheus_remote_write" do
        assert_true @serde.from_msgpack(@buffer)
        assert_not_nil @serde.prometheus_remote_write
      end

      test "encode text" do
        assert_true @serde.from_msgpack(@buffer)
        assert_not_nil @serde.to_s
      end

      test "encode influx" do
        assert_true @serde.from_msgpack(@buffer)
        expected = <<-EOC
kubernetes_network load=1 \\d+
kubernetes_network,hostname=calyptia.com,app=cmetrics load=2 \\d+
EOC
        assert_match(/#{expected}/, @serde.to_influx)
      end

      test "encode prometheus" do
        assert_true @serde.from_msgpack(@buffer)
        expected = <<-EOC
# HELP kubernetes_network_load Network load
# TYPE kubernetes_network_load counter
kubernetes_network_load 1 \\d+
kubernetes_network_load{hostname=\"calyptia.com\",app=\"cmetrics\"} 2 \\d+
EOC
        assert_match(/#{expected}/, @serde.to_prometheus)
      end

      test "decode as Hash" do
        assert_true @serde.from_msgpack(@buffer)
        expected = [
          {"namespace"=>"kubernetes", "subsystem"=>"network", "name"=>"load", "description"=>"Network load", "value"=>1.0},
          {"namespace"=>"kubernetes", "subsystem"=>"network", "labels"=>{"hostname"=>"calyptia.com", "app"=>"cmetrics"}, "name"=>"load", "description"=>"Network load", "value"=>2.0}
        ]
        assert_equal([[Float], [Float]], @serde.metrics.first.map{|e| e.select{|k| k == "timestamp"}.values.map{|e| e.class}})
        assert_equal(expected, @serde.metrics.first.map{|e| e.reject!{|k| k == "timestamp"}})
      end

      sub_test_case "w/ static labels" do
        setup do
          @counter = CMetrics::Counter.new
          @counter.create("kubernetes", "network", "load", "Network load", ["hostname", "app"])
          @counter.inc
          @counter.inc(["calyptia.com", "cmetrics"])
          @counter.inc(["calyptia.com", "cmetrics"])
          @counter.add_label("dev", "Calyptia")
          @counter.add_label("lang", "Ruby")
          @buffer = @counter.to_msgpack
        end

        test "decode as Hash" do
          assert_true @serde.from_msgpack(@buffer)
          expected = [
            {"namespace"=>"kubernetes", "subsystem"=>"network", "name"=>"load",
             "static_labels"=>{"dev"=>"Calyptia", "lang"=>"Ruby"}, "description"=>"Network load", "value"=>1.0},
            {"namespace"=>"kubernetes", "subsystem"=>"network", "labels"=>{"hostname"=>"calyptia.com", "app"=>"cmetrics"},
             "static_labels"=>{"dev"=>"Calyptia", "lang"=>"Ruby"},
             "name"=>"load", "description"=>"Network load", "value"=>2.0}
          ]
          assert_equal([[Float], [Float]], @serde.metrics.first.map{|e| e.select{|k| k == "timestamp"}.values.map{|e| e.class}})
          assert_equal(expected, @serde.metrics.first.map{|e| e.reject!{|k| k == "timestamp"}})
        end
      end
    end

    sub_test_case "w/ gauge" do
      setup do
        @gauge = CMetrics::Gauge.new
        @gauge.create("kubernetes", "network", "load", "Network load", ["hostname", "app"])
        @gauge.inc
        @gauge.inc(["calyptia.com", "cmetrics"])
        @gauge.inc(["calyptia.com", "cmetrics"])
        @buffer = @gauge.to_msgpack
      end

      test "decode gauge" do
        assert_true @serde.from_msgpack(@buffer)
        buffer = @serde.to_msgpack
        assert_equal buffer.size, @buffer.size
      end

      test "prometheus_remote_write" do
        assert_true @serde.from_msgpack(@buffer)
        assert_not_nil @serde.prometheus_remote_write
      end

      test "encode text" do
        assert_true @serde.from_msgpack(@buffer)
        assert_not_nil @serde.to_s
      end

      test "encode influx" do
        assert_true @serde.from_msgpack(@buffer)
        expected = <<-EOC
kubernetes_network load=1 \\d+
kubernetes_network,hostname=calyptia.com,app=cmetrics load=2 \\d+
EOC
        assert_match(/#{expected}/, @serde.to_influx)
      end

      test "encode prometheus" do
        assert_true @serde.from_msgpack(@buffer)
        expected = <<-EOC
# HELP kubernetes_network_load Network load
# TYPE kubernetes_network_load gauge
kubernetes_network_load 1 \\d+
kubernetes_network_load{hostname=\"calyptia.com\",app=\"cmetrics\"} 2 \\d+
EOC
        assert_match(/#{expected}/, @serde.to_prometheus)
      end

      test "decode as Hash" do
        assert_true @serde.from_msgpack(@buffer)
        expected = [
          {"namespace"=>"kubernetes", "subsystem"=>"network", "name"=>"load", "description"=>"Network load", "value"=>1.0},
          {"namespace"=>"kubernetes", "subsystem"=>"network", "labels"=>{"hostname"=>"calyptia.com", "app"=>"cmetrics"}, "name"=>"load", "description"=>"Network load", "value"=>2.0}
        ]
        assert_equal([[Float], [Float]], @serde.metrics.first.map{|e| e.select{|k| k == "timestamp"}.values.map{|e| e.class}})
        assert_equal(expected, @serde.metrics.first.map{|e| e.reject!{|k| k == "timestamp"}})
      end

      sub_test_case "w/ static labels" do
        setup do
          @gauge = CMetrics::Gauge.new
          @gauge.create("kubernetes", "network", "load", "Network load", ["hostname", "app"])
          @gauge.inc
          @gauge.inc(["calyptia.com", "cmetrics"])
          @gauge.inc(["calyptia.com", "cmetrics"])
          @gauge.add_label("dev", "Calyptia")
          @gauge.add_label("lang", "Ruby")
          @buffer = @gauge.to_msgpack
        end

        test "decode as Hash" do
          assert_true @serde.from_msgpack(@buffer)
          expected = [
            {"namespace"=>"kubernetes", "subsystem"=>"network", "name"=>"load",
             "static_labels"=>{"dev"=>"Calyptia", "lang"=>"Ruby"}, "description"=>"Network load", "value"=>1.0},
            {"namespace"=>"kubernetes", "subsystem"=>"network", "labels"=>{"hostname"=>"calyptia.com", "app"=>"cmetrics"},
             "static_labels"=>{"dev"=>"Calyptia", "lang"=>"Ruby"},
             "name"=>"load", "description"=>"Network load", "value"=>2.0}
          ]
          assert_equal([[Float], [Float]], @serde.metrics.first.map{|e| e.select{|k| k == "timestamp"}.values.map{|e| e.class}})
          assert_equal(expected, @serde.metrics.first.map{|e| e.reject!{|k| k == "timestamp"}})
        end
      end
    end

    sub_test_case "concat objects and decode msgpack in a bundle" do
      setup do
        @counter = CMetrics::Counter.new
        @counter.create("test", "concat", "counter", "Dest counter data")
        @counter.set(1)
        @buffer = @counter.to_msgpack
      end

      data(
        counter: ["counter", CMetrics::Counter.new],
        gauge: ["gauge", CMetrics::Counter.new],
        untyped: ["untyped", CMetrics::Untyped.new]
      )
      test "concatenate cmetric object" do |(label, obj)|
        assert_true @serde.from_msgpack(@buffer)

        obj.create("test", "concat", label, "Source #{label} data")
        obj.set(10)
        @serde.concat(obj)

        unpacked = MessagePack.unpack(@serde.to_msgpack)
        assert_equal([
                       2,
                       {"ns"=>"test", "ss"=>"concat", "name"=>"counter", "desc"=>"Dest counter data"},
                       {"ns"=>"test", "ss"=>"concat", "name"=>label, "desc"=>"Source #{label} data"},
                       1.0,
                       10.0
                     ],
                     [
                       unpacked.size,
                       unpacked.first["meta"]["opts"],
                       unpacked.last["meta"]["opts"],
                       unpacked.first["values"].last["value"],
                       unpacked.last["values"].last["value"]
                     ])
      end

      data(
        counter: ["counter", CMetrics::Counter.new],
        gauge: ["gauge", CMetrics::Counter.new],
        untyped: ["untyped", CMetrics::Untyped.new]
      )
      test "concatenate cmetric object without from_msgpack first" do |(label, obj)|
        obj.create("test", "concat", label, "Source #{label} data")
        obj.set(10)
        @serde.concat(obj)
        unpacked = MessagePack.unpack(@serde.to_msgpack)
        assert_equal([
                       1,
                       {"ns"=>"test", "ss"=>"concat", "name"=>label, "desc"=>"Source #{label} data"},
                       10.0
                     ],
                     [
                       unpacked.size,
                       unpacked.first["meta"]["opts"],
                       unpacked.first["values"].last["value"],
                     ])
      end

      test "concatenate many cmetric objects" do
        10.times do |i|
          @counter = CMetrics::Counter.new
          @counter.create("test", "concat", "counter", "Source #{i} data")
          @counter.set(i)
          @serde.concat(@counter)
        end
        unpacked = MessagePack.unpack(@serde.to_msgpack)
        opts = unpacked.collect do |obj|
          obj["meta"]["opts"]
        end
        values = unpacked.collect do |obj|
          obj["values"].first["value"]
        end
        assert_equal([
                       10,
                       [
                         {"ns"=>"test", "ss"=>"concat", "name"=>"counter", "desc"=>"Source 0 data"},
                         {"ns"=>"test", "ss"=>"concat", "name"=>"counter", "desc"=>"Source 1 data"},
                         {"ns"=>"test", "ss"=>"concat", "name"=>"counter", "desc"=>"Source 2 data"},
                         {"ns"=>"test", "ss"=>"concat", "name"=>"counter", "desc"=>"Source 3 data"},
                         {"ns"=>"test", "ss"=>"concat", "name"=>"counter", "desc"=>"Source 4 data"},
                         {"ns"=>"test", "ss"=>"concat", "name"=>"counter", "desc"=>"Source 5 data"},
                         {"ns"=>"test", "ss"=>"concat", "name"=>"counter", "desc"=>"Source 6 data"},
                         {"ns"=>"test", "ss"=>"concat", "name"=>"counter", "desc"=>"Source 7 data"},
                         {"ns"=>"test", "ss"=>"concat", "name"=>"counter", "desc"=>"Source 8 data"},
                         {"ns"=>"test", "ss"=>"concat", "name"=>"counter", "desc"=>"Source 9 data"},
                       ],
                       [0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0]
                     ],
                     [
                       unpacked.size,
                       opts,
                       values
                     ])
      end
    end

    sub_test_case "w/ wired buffer" do
      setup do
        @gauge = CMetrics::Gauge.new
        @gauge.create("kubernetes", "network", "load", "Network load", ["hostname", "app"])
        @gauge.set 2.0
        @gauge.inc(["localhost", "cmetrics"])
        @gauge.add(10, ["localhost", "test"])
        @counter = CMetrics::Counter.new
        @counter.create("kubernetes", "network", "load", "Network load", ["hostname", "app"])
        @counter.inc
        @counter.inc(["localhost", "cmetrics"])
        @counter.add(10.50, ["localhost", "test"])
        @wired_buffer = @gauge.to_msgpack + @counter.to_msgpack
      end

      sub_test_case "decode wired buffer" do
        test "#feed_each" do
          serdes = []
          @serde.feed_each(@wired_buffer) do |serde|
            serdes << serde.to_influx
          end
          assert_equal @gauge.to_influx, serdes[0]
          # Trying to decode from the next offset.
          assert_equal @counter.to_influx, serdes[1]
        end

        test "explicit offset arguments" do
          assert_true @serde.from_msgpack(@wired_buffer, @wired_buffer.size)
          buffer = @serde.to_msgpack
          assert_equal @gauge.to_influx, @serde.to_influx
          # Trying to decode from the next offset.
          assert_true @serde.from_msgpack(@wired_buffer, @wired_buffer.size, buffer.size)
          assert_equal @counter.to_influx, @serde.to_influx
        end

        test "implicit offset arguments" do
          assert_true @serde.from_msgpack(@wired_buffer, @wired_buffer.size)
          buffer = @serde.to_msgpack
          assert_equal @gauge.to_influx, @serde.to_influx
          # Trying to decode from the next offset.
          assert_true @serde.from_msgpack(@wired_buffer)
          assert_equal @counter.to_influx, @serde.to_influx
        end
      end

      sub_test_case "decode and encode as prometheus remote write" do
        test "prometheus remote with multiple cmetrics objects" do
          encoded_buffer = ""
          @serde.feed_each(@wired_buffer) do |serde|
            encoded_buffer << serde.prometheus_remote_write
          end
          assert_not_nil encoded_buffer
        end
      end

      sub_test_case "encode text" do
        test "#feed_each" do
          texts = []
          @serde.feed_each(@wired_buffer) do |serde|
            texts << serde.to_s
          end
          assert_not_nil texts[0].to_s
          # Trying to decode from the next offset.
          assert_not_nil texts[1].to_s
        end

        test "explicit offset arguments" do
          assert_true @serde.from_msgpack(@wired_buffer)
          buffer = @serde.to_msgpack
          assert_not_nil @serde.to_s
          # Trying to decode from the next offset.
          assert_true @serde.from_msgpack(@wired_buffer, @wired_buffer.size, buffer.size)
          assert_not_nil @serde.to_s
        end

        test "implicit offset arguments" do
          assert_true @serde.from_msgpack(@wired_buffer)
          buffer = @serde.to_msgpack
          assert_not_nil @serde.to_s
          # Trying to decode from the next offset.
          assert_true @serde.from_msgpack(@wired_buffer)
          assert_not_nil @serde.to_s
        end
      end

      sub_test_case "encode influx" do
        test "#feed_each" do
          encoded_influxes = []
          @serde.feed_each(@wired_buffer) do |serde|
            encoded_influxes << serde.to_influx
          end
          expected_gauge = <<-EOC
kubernetes_network load=2 \\d+
kubernetes_network,hostname=localhost,app=cmetrics load=1 \\d+
kubernetes_network,hostname=localhost,app=test load=10 \\d+
EOC
          expected_counter = <<EOF
kubernetes_network load=1 \\d+
kubernetes_network,hostname=localhost,app=cmetrics load=1 \\d+
kubernetes_network,hostname=localhost,app=test load=10.5 \\d+
EOF
          assert_match(/#{expected_gauge}/, encoded_influxes[0])
          # Trying to decode from the next offset.
          assert_match(/#{expected_counter}/, encoded_influxes[1])
        end

        test "explicit offset arguments" do
          assert_true @serde.from_msgpack(@wired_buffer, @wired_buffer.size)
          decoded_gauge_buffer = @serde.to_msgpack
          expected_gauge = <<-EOC
kubernetes_network load=2 \\d+
kubernetes_network,hostname=localhost,app=cmetrics load=1 \\d+
kubernetes_network,hostname=localhost,app=test load=10 \\d+
EOC
          expected_counter = <<EOF
kubernetes_network load=1 \\d+
kubernetes_network,hostname=localhost,app=cmetrics load=1 \\d+
kubernetes_network,hostname=localhost,app=test load=10.5 \\d+
EOF
          assert_match(/#{expected_gauge}/, @serde.to_influx)
          # Trying to decode from the next offset.
          assert_true @serde.from_msgpack(@wired_buffer, @wired_buffer.size, decoded_gauge_buffer.size)
          assert_match(/#{expected_counter}/, @serde.to_influx)
        end

        test "implicit offset arguments" do
          assert_true @serde.from_msgpack(@wired_buffer, @wired_buffer.size)
          expected_gauge = <<-EOC
kubernetes_network load=2 \\d+
kubernetes_network,hostname=localhost,app=cmetrics load=1 \\d+
kubernetes_network,hostname=localhost,app=test load=10 \\d+
EOC
          expected_counter = <<EOF
kubernetes_network load=1 \\d+
kubernetes_network,hostname=localhost,app=cmetrics load=1 \\d+
kubernetes_network,hostname=localhost,app=test load=10.5 \\d+
EOF
          assert_match(/#{expected_gauge}/, @serde.to_influx)
          # Trying to decode from the next offset.
          assert_true @serde.from_msgpack(@wired_buffer)
          assert_match(/#{expected_counter}/, @serde.to_influx)
        end
      end

      sub_test_case "encode prometheus" do
        test "#feed_each" do
          encoded_prometheuses = []
          @serde.feed_each(@wired_buffer) do |serde|
            encoded_prometheuses << serde.to_prometheus
          end
          expected_gauge = <<-EOC
# HELP kubernetes_network_load Network load
# TYPE kubernetes_network_load gauge\nkubernetes_network_load 2 \\d+
kubernetes_network_load{hostname=\"localhost\",app=\"cmetrics\"} 1 \\d+
kubernetes_network_load{hostname=\"localhost\",app=\"test\"} 10 \\d+
EOC
          expected_counter = <<-EOF
# HELP kubernetes_network_load Network load
# TYPE kubernetes_network_load counter
kubernetes_network_load 1 \\d+
kubernetes_network_load{hostname=\"localhost\",app=\"cmetrics\"} 1 \\d+
kubernetes_network_load{hostname=\"localhost\",app=\"test\"} 10.5 \\d+
EOF
          assert_match(/#{expected_gauge}/, encoded_prometheuses[0])
          # Trying to decode from the next offset.
          assert_match(/#{expected_counter}/, encoded_prometheuses[1])
        end

        test "explicit offset arguments" do
          assert_true @serde.from_msgpack(@wired_buffer, @wired_buffer.length)
          decoded_gauge_buffer = @serde.to_msgpack
          expected_gauge = <<-EOC
# HELP kubernetes_network_load Network load
# TYPE kubernetes_network_load gauge\nkubernetes_network_load 2 \\d+
kubernetes_network_load{hostname=\"localhost\",app=\"cmetrics\"} 1 \\d+
kubernetes_network_load{hostname=\"localhost\",app=\"test\"} 10 \\d+
EOC
          expected_counter = <<-EOF
# HELP kubernetes_network_load Network load
# TYPE kubernetes_network_load counter
kubernetes_network_load 1 \\d+
kubernetes_network_load{hostname=\"localhost\",app=\"cmetrics\"} 1 \\d+
kubernetes_network_load{hostname=\"localhost\",app=\"test\"} 10.5 \\d+
EOF
          assert_match(/#{expected_gauge}/, @serde.to_prometheus)
          # Trying to decode from the next offset.
          assert_true @serde.from_msgpack(@wired_buffer, @wired_buffer.size, decoded_gauge_buffer.size)
          assert_match(/#{expected_counter}/, @serde.to_prometheus)
        end

        test "implicit offset arguments" do
          assert_true @serde.from_msgpack(@wired_buffer, @wired_buffer.length)
          expected_gauge = <<-EOC
# HELP kubernetes_network_load Network load
# TYPE kubernetes_network_load gauge\nkubernetes_network_load 2 \\d+
kubernetes_network_load{hostname=\"localhost\",app=\"cmetrics\"} 1 \\d+
kubernetes_network_load{hostname=\"localhost\",app=\"test\"} 10 \\d+
EOC
          expected_counter = <<-EOF
# HELP kubernetes_network_load Network load
# TYPE kubernetes_network_load counter
kubernetes_network_load 1 \\d+
kubernetes_network_load{hostname=\"localhost\",app=\"cmetrics\"} 1 \\d+
kubernetes_network_load{hostname=\"localhost\",app=\"test\"} 10.5 \\d+
EOF
          assert_match(/#{expected_gauge}/, @serde.to_prometheus)
          # Trying to decode from the next offset.
          assert_true @serde.from_msgpack(@wired_buffer)
          assert_match(/#{expected_counter}/, @serde.to_prometheus)
        end
      end

      sub_test_case "decode as Hash" do
        test "decode as Hash" do
          assert_true @serde.from_msgpack(@wired_buffer)

          expected = [
            {"namespace"=>"kubernetes", "subsystem"=>"network", "name"=>"load", "description"=>"Network load", "value"=>2.0},
            {"namespace"=>"kubernetes", "subsystem"=>"network", "labels"=>{"hostname"=>"localhost", "app"=>"cmetrics"}, "name"=>"load", "description"=>"Network load", "value"=>1.0},
            {"namespace"=>"kubernetes", "subsystem"=>"network", "labels"=>{"hostname"=>"localhost", "app"=>"test"}, "name"=>"load", "description"=>"Network load", "value"=>10.0}
          ]
          assert_equal([[Float], [Float], [Float]], @serde.metrics.first.map{|e| e.select{|k| k == "timestamp"}.values.map{|e| e.class}})
        assert_equal(expected, @serde.metrics.first.map{|e| e.reject!{|k| k == "timestamp"}})
        end
      end
    end
  end
end
