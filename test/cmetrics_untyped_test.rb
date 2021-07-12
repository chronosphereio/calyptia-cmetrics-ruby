# frozen_string_literal: true

require "test_helper"

class CMetricsUntypedTest < Test::Unit::TestCase
  sub_test_case "untyped" do
    setup do
      @untyped = CMetrics::Untyped.new
      @untyped.create("kubernetes", "network", "load", "Network load", ["hostname", "app"])
    end

    def test_untyped
      assert_equal nil, @untyped.val

      assert_true @untyped.set 3.0
      assert_equal 3.0, @untyped.val
      expected = <<-EOC
# HELP kubernetes_network_load Network load
# TYPE kubernetes_network_load untyped
kubernetes_network_load 3 \\d+
EOC
      assert_match(/#{expected}/, @untyped.to_prometheus)
      assert_not_nil @untyped.to_s
    end

    def test_labels
      assert_true @untyped.set(1, ["localhost", "cmetrics"])
      assert_equal 1.0, @untyped.val(["localhost", "cmetrics"])

      assert_true @untyped.set(12.15, ["localhost", "test"])
      expected = <<-EOC
# HELP kubernetes_network_load Network load
# TYPE kubernetes_network_load untyped
kubernetes_network_load{hostname="localhost",app="cmetrics"} 1 \\d+
kubernetes_network_load{hostname="localhost",app="test"} 12.15 \\d+
EOC
      assert_match(/#{expected}/, @untyped.to_prometheus)
      assert_not_nil @untyped.to_s
    end

    def test_prometheus
      untyped = CMetrics::Untyped.new
      untyped.create("cmt", "labels", "test", "Static labels test", ["host", "app"])

      untyped.set 1
      untyped.set(2, ["calyptia.com", "cmetrics"])

      expected = <<-EOC
# HELP cmt_labels_test Static labels test
# TYPE cmt_labels_test untyped
cmt_labels_test 1 \\d+
cmt_labels_test{host="calyptia.com",app="cmetrics"} 2 \\d+
EOC
      assert_match(/#{expected}/, untyped.to_prometheus)

      assert_true untyped.add_label("dev", "Calyptia")
      assert_true untyped.add_label("lang", "C")

      expected2 = <<-EOC
# HELP cmt_labels_test Static labels test
# TYPE cmt_labels_test untyped
cmt_labels_test{dev="Calyptia",lang="C"} 1 \\d+
cmt_labels_test{dev="Calyptia",lang="C",host="calyptia.com",app="cmetrics"} 2 \\d+
EOC
      assert_match(/#{expected2}/, untyped.to_prometheus)
    end

    def test_influx
      untyped = CMetrics::Untyped.new
      untyped.create("cmt", "labels", "test", "Static labels test", ["host", "app"])

      untyped.set 1.0
      untyped.set(2.0, ["calyptia.com", "cmetrics"])

      expected = <<-EOC
cmt_labels test=1 \\d+
cmt_labels,host=calyptia.com,app=cmetrics test=2 \\d+
EOC
      assert_match(/#{expected}/, untyped.to_influx)

      assert_true untyped.add_label("dev", "Calyptia")
      assert_true untyped.add_label("lang", "C")

      expected2 = <<-EOC
cmt_labels,dev=Calyptia,lang=C test=1 \\d+
cmt_labels,dev=Calyptia,lang=C,host=calyptia.com,app=cmetrics test=2 \\d+
EOC
      assert_match(/#{expected2}/, untyped.to_influx)
    end
  end

  sub_test_case "untyped w/ one symbol" do
    setup do
      @untyped = CMetrics::Untyped.new
      @untyped.create("kubernetes", "network", "load", "Network load", :hostname)
    end

    def test_untyped
      assert_equal nil, @untyped.val

      assert_true @untyped.set 3.0
      assert_equal 3.0, @untyped.val
      expected = <<-EOC
# HELP kubernetes_network_load Network load
# TYPE kubernetes_network_load untyped
kubernetes_network_load 3 \\d+
EOC
      assert_match(/#{expected}/, @untyped.to_prometheus)
      assert_not_nil @untyped.to_s
    end

    def test_label
      assert_true @untyped.set(1.0, :localhost)
      assert_equal 1.0, @untyped.val(:localhost)

      assert_true @untyped.set(12.15, :k8s_worker)
      assert_false @untyped.set(5.5, :k8s_worker)
      expected = <<-EOC
# HELP kubernetes_network_load Network load
# TYPE kubernetes_network_load untyped
kubernetes_network_load{hostname="localhost"} 1 \\d+
kubernetes_network_load{hostname="k8s_worker"} 12.15 \\d+
EOC
      assert_match(/#{expected}/, @untyped.to_prometheus)
      assert_not_nil @untyped.to_s
    end
  end

  sub_test_case "untyped w/ symbols" do
    setup do
      @untyped = CMetrics::Untyped.new
      @untyped.create("kubernetes", "network", "load", "Network load", [:hostname, :app])
    end

    def test_untyped
      assert_equal nil, @untyped.val

      assert_true @untyped.set 3.0
      assert_equal 3.0, @untyped.val
      expected = <<-EOC
# HELP kubernetes_network_load Network load
# TYPE kubernetes_network_load untyped
kubernetes_network_load 3 \\d+
EOC
      assert_match(/#{expected}/, @untyped.to_prometheus)
      assert_not_nil @untyped.to_s
    end

    def test_labels
      assert_true @untyped.set(1.0, [:localhost, :cmetrics])
      assert_equal 1.0, @untyped.val([:localhost, :cmetrics])

      assert_true @untyped.set(12.15, [:localhost, :test])
      assert_false @untyped.set(2, [:localhost, :test])
      expected = <<-EOC
# HELP kubernetes_network_load Network load
# TYPE kubernetes_network_load untyped
kubernetes_network_load{hostname="localhost",app="cmetrics"} 1 \\d+
kubernetes_network_load{hostname="localhost",app="test"} 12.15 \\d+
EOC
      assert_match(/#{expected}/, @untyped.to_prometheus)
      assert_not_nil @untyped.to_msgpack
      assert_not_nil @untyped.to_s
    end
  end

  sub_test_case "not enough initialized" do
    setup do
      @untyped = CMetrics::Untyped.new
    end

    def test_error
      assert_raise(NoMethodError) do
        @untyped.inc
      end

      assert_raise(NoMethodError) do
        @untyped.add 2.0
      end

      assert_raise(RuntimeError) do
        @untyped.set 2.0
      end

      assert_raise(RuntimeError) do
        @untyped.val
      end
    end
  end
end
