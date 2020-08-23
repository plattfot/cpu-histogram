(use-modules
  (guix packages)
  ((guix licenses) #:prefix license:)
  (guix build-system meson)
  (guix gexp)
  (gnu packages)
  (gnu packages serialization)
  (gnu packages pretty-print)
  (gnu packages pkg-config))

(package
  (name "waybar-cpu-histogram")
  (version "0.2.0")
  (source (local-file (dirname (dirname (current-filename))) #:recursive? #t))
  (build-system meson-build-system)
  (native-inputs
   `(("pkg-config" ,pkg-config)))
  (inputs
    `(("jsoncpp" ,jsoncpp)
      ("fmt" ,fmt)))
  (synopsis "CPU histogram for waybar")
  (description
    "Custom module for waybar to show CPU usage as a histogram. Which
is a more compact way to see how many cores are active than have a bar
for each CPU.")
  (home-page
    "https://github.com/plattfot/cpu-histogram/")
  (license license:expat))

