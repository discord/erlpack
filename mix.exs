defmodule ErlPack.Mixfile do
  use Mix.Project

  def project do
    [
      app: :erlpack,
      version: "1.0.0",
      elixir: "~> 1.3",
      build_embedded: Mix.env == :prod,
      start_permanent: Mix.env == :prod,
      deps: deps(),
      package: package(),
      elixirc_paths: ["ex/erlpack/lib/"],
      test_paths: ["ex/erlpack/test/"]
    ]
  end

  def application do
    [
      applications: []
    ]
  end

  defp deps do
    []
  end

  def package do
    [
      name: :erlpack,
      description: "High Performance Erlang Term Format Packer",
      maintainers: [],
      licenses: ["MIT"],
      files: ["ex/erlpack/lib/*", "mix.exs", "README*", "LICENSE*"],
      links: %{
        "GitHub" => "https://github.com/discordapp/erlpack",
      },
    ]
  end
end