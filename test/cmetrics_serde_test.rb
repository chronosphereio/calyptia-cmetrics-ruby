# frozen_string_literal: true

require "test_helper"

class CMetricsSerdeTest < Test::Unit::TestCase
  sub_test_case "Serde" do
    setup do
      @serde = CMetrics::Serde.new
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
        @counter.add(10.55, ["localhost", "test"])
        @wired_buffer = @gauge.to_msgpack + @counter.to_msgpack
      end

      test "decode wired buffer" do
        assert_true @serde.from_msgpack(@wired_buffer)
        buffer = @serde.to_msgpack
        assert_equal buffer.size, @wired_buffer.size
      end
    end
  end
end
