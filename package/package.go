package rdno_telnet

import (
	denv "github.com/jurgen-kluft/ccode/denv"
	rdno_core "github.com/jurgen-kluft/rdno_core/package"
)

// rdno_telnet is a debug package for Arduino Esp32/Esp8266 projects.
const (
	repo_path = "github.com\\jurgen-kluft"
	repo_name = "rdno_telnet"
)

func GetPackage() *denv.Package {
	name := repo_name

	// dependencies
	corepkg := rdno_core.GetPackage()

	// main package
	mainpkg := denv.NewPackage(repo_path, repo_name)
	mainpkg.AddPackage(corepkg)

	// main library
	mainlib := denv.SetupCppLibProject(mainpkg, name)
	mainlib.AddDependencies(corepkg.GetMainLib())

	mainpkg.AddMainLib(mainlib)
	return mainpkg
}
