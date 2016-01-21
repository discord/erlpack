#!/usr/bin/env elixir

ExUnit.start

defmodule EchoClientMacros do
  defmacro testcase(name, term) do
    quote do
      test unquote(name), %{socket: socket} do
        test_term(socket, unquote(name), unquote(term))
      end
    end
  end
end

defmodule EchoClient do
  @port 5001

  import EchoClientMacros

  use ExUnit.Case, async: false, seed: 0

  setup_all do 
    opts = [:binary, active: false, packet: 4, nodelay: true]
    {:ok, socket} = :gen_tcp.connect('localhost', @port, opts)
    {:ok, %{socket: socket}}
  end

  def test_term(socket, name, term) do
    :ok = :gen_tcp.send(socket, :erlang.term_to_binary(name))
    {:ok, packet} = :gen_tcp.recv(socket, 0)
    binary = :erlang.term_to_binary(term)
    :ok = :gen_tcp.send(socket, binary)
    {:ok, packet} = :gen_tcp.recv(socket, 0)
    returned_term = :erlang.binary_to_term(packet)
    assert(term === returned_term, "Returned term doesn't match")
  end

  testcase "basic_atom", :hi
  testcase "empty_list", []
  testcase "empty_dictionary", {}
  testcase "string", "string"
  testcase "binary", <<"alsdjaljf">>
  testcase "int", 12345
  testcase "float", 123.45
  testcase "large_int", 127552384489488384
  testcase "kitchen_sink", [
    :someatom,
    {:some, :other, "tuple"},
    ["maybe", 1, []],
    {"with", {:embedded, ["tuples and lists"]}, nil},
    127542384389482384,
    5334.32,
    102,
    -1394,
    -349.2,
    -498384595043,
    [%{a: "map", with: <<"binaries">>, also: {<<"tuples">>, ["and"], ["lists"]}},
     %{:a => "anotherone", 3 => "int keys"}],
    %{{:something} => "else"}
  ]
  # testcase "really large int", 12345678901234512309301923091 # Currently not supported

end
