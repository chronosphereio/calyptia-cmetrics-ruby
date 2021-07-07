# frozen_string_literal: true

require "test_helper"

class CMetricsGaugeTest < Test::Unit::TestCase
  sub_test_case "gauge" do
    setup do
      @gauge = CMetrics::Gauge.new
      @gauge.create("kubernetes", "network", "load", "Network load", ["hostname", "app"])
    end

    def test_gauge
      assert_equal nil, @gauge.val
      assert_true @gauge.set 2.0

      assert_equal 2.0, @gauge.val

      assert_true @gauge.inc
      assert_equal 3.0, @gauge.val

      assert_true @gauge.sub 2.0
      assert_equal 1.0, @gauge.val

      assert_true @gauge.dec
      assert_equal 0.0, @gauge.val
      expected = <<-EOC
# HELP kubernetes_network_load Network load
# TYPE kubernetes_network_load gauge
kubernetes_network_load 0 \\d+
EOC
      assert_match(/#{expected}/, @gauge.to_prometheus)
      assert_not_nil @gauge.to_s
    end

    def test_labels
      assert_true @gauge.inc(["localhost", "cmetrics"])
      assert_equal 1.0, @gauge.val(["localhost", "cmetrics"])

      assert_true @gauge.add(10, ["localhost", "test"])
      assert_equal 10, @gauge.val(["localhost", "test"])

      assert_true @gauge.sub(2.5, ["localhost", "test"])
      assert_equal 7.5, @gauge.val(["localhost", "test"])

      expected = <<-EOC
# HELP kubernetes_network_load Network load
# TYPE kubernetes_network_load gauge
kubernetes_network_load{hostname="localhost",app="cmetrics"} 1 \\d+
kubernetes_network_load{hostname="localhost",app="test"} 7.5 \\d+
EOC
      assert_match(/#{expected}/, @gauge.to_prometheus)
      assert_not_nil @gauge.to_msgpack
    end

    def test_prometheus
      gauge = CMetrics::Gauge.new
      gauge.create("cmt", "labels", "test", "Static labels test", ["host", "app"])

      gauge.inc
      gauge.inc(["calyptia.com", "cmetrics"])
      gauge.inc(["calyptia.com", "cmetrics"])

      expected = <<-EOC
# HELP cmt_labels_test Static labels test
# TYPE cmt_labels_test gauge
cmt_labels_test 1 \\d+
cmt_labels_test{host="calyptia.com",app="cmetrics"} 2 \\d+
EOC
      assert_match(/#{expected}/, gauge.to_prometheus)

      assert_true gauge.add_label("dev", "Calyptia")
      assert_true gauge.add_label("lang", "C")

      expected2 = <<-EOC
# HELP cmt_labels_test Static labels test
# TYPE cmt_labels_test gauge
cmt_labels_test{dev="Calyptia",lang="C"} 1 \\d+
cmt_labels_test{dev="Calyptia",lang="C",host="calyptia.com",app="cmetrics"} 2 \\d+
EOC
      assert_match(/#{expected2}/, gauge.to_prometheus)
    end

    def test_influx
      gauge = CMetrics::Gauge.new
      gauge.create("cmt", "labels", "test", "Static labels test", ["host", "app"])

      gauge.inc
      gauge.inc(["calyptia.com", "cmetrics"])
      gauge.inc(["calyptia.com", "cmetrics"])

      expected = <<-EOC
cmt_labels test=1 \\d+
cmt_labels,host=calyptia.com,app=cmetrics test=2 \\d+
EOC
      assert_match(/#{expected}/, gauge.to_influx)

      assert_true gauge.add_label("dev", "Calyptia")
      assert_true gauge.add_label("lang", "C")

      expected2 = <<-EOC
cmt_labels,dev=Calyptia,lang=C test=1 \\d+
cmt_labels,dev=Calyptia,lang=C,host=calyptia.com,app=cmetrics test=2 \\d+
EOC
      assert_match(/#{expected2}/, gauge.to_influx)
    end
  end

  sub_test_case "gauge w/ one symbol" do
    setup do
      @gauge = CMetrics::Gauge.new
      @gauge.create("kubernetes", "network", "load", "Network load", :hostname)
    end

    def test_gauge
      assert_equal nil, @gauge.val
      assert_true @gauge.inc
      assert_equal 1.0, @gauge.val

      assert_true @gauge.add 2.0
      assert_equal 3.0, @gauge.val
      expected = <<-EOC
# HELP kubernetes_network_load Network load
# TYPE kubernetes_network_load gauge
kubernetes_network_load 3 \\d+
EOC
      assert_match(/#{expected}/, @gauge.to_prometheus)
      assert_not_nil @gauge.to_s
    end

    def test_label
      assert_true @gauge.inc(:localhost)
      assert_equal 1.0, @gauge.val(:localhost)

      assert_true @gauge.add(10.55, :k8s_worker)
      assert_equal 10.55, @gauge.val(:k8s_worker)

      assert_true @gauge.set(12.15, :k8s_worker)
      assert_true @gauge.set(1, :k8s_worker)
      expected = <<-EOC
# HELP kubernetes_network_load Network load
# TYPE kubernetes_network_load gauge
kubernetes_network_load{hostname="localhost"} 1 \\d+
kubernetes_network_load{hostname="k8s_worker"} 1 \\d+
EOC
      assert_match(/#{expected}/, @gauge.to_prometheus)
      assert_not_nil @gauge.to_s
    end
  end

  sub_test_case "gauge w/ symbols" do
    setup do
      @gauge = CMetrics::Gauge.new
      @gauge.create("kubernetes", "network", "load", "Network load", [:hostname, :app])
    end

    def test_gauge
      assert_equal nil, @gauge.val
      assert_true @gauge.set 2.0

      assert_equal 2.0, @gauge.val

      assert_true @gauge.inc
      assert_equal 3.0, @gauge.val

      assert_true @gauge.sub 2.0
      assert_equal 1.0, @gauge.val

      assert_true @gauge.dec
      assert_equal 0.0, @gauge.val
      expected = <<-EOC
# HELP kubernetes_network_load Network load
# TYPE kubernetes_network_load gauge
kubernetes_network_load 0 \\d+
EOC
      assert_match(/#{expected}/, @gauge.to_prometheus)
      assert_not_nil @gauge.to_s
    end

    def test_labels
      assert_true @gauge.inc([:localhost, :cmetrics])
      assert_equal 1.0, @gauge.val([:localhost, :cmetrics])

      assert_true @gauge.add(10, [:localhost, :test])
      assert_equal 10, @gauge.val([:localhost, :test])

      assert_true @gauge.sub(2.5, [:localhost, :test])
      assert_equal 7.5, @gauge.val([:localhost, :test])

      expected = <<-EOC
# HELP kubernetes_network_load Network load
# TYPE kubernetes_network_load gauge
kubernetes_network_load{hostname="localhost",app="cmetrics"} 1 \\d+
kubernetes_network_load{hostname="localhost",app="test"} 7.5 \\d+
EOC
      assert_match(/#{expected}/, @gauge.to_prometheus)
      assert_not_nil @gauge.to_s
    end
  end
end
