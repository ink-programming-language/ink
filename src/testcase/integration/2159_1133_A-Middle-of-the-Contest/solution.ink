// Translated from solution.cpp.

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

var d: dynamic = cpp_uninitialized();

var i: dynamic = cpp_uninitialized();

var e: dynamic = cpp_uninitialized();

var f: dynamic = cpp_uninitialized();

var g: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var l: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

var s1: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(s, s1);
  a += ((((((((((int_cpp(s[0]) - 48)) * 10) + int_cpp(s[1])) - 48)) * 60) + (((int_cpp(s[3]) - 48)) * 10)) + int_cpp(s[4])) - 48);
  b += ((((((((((int_cpp(s1[0]) - 48)) * 10) + int_cpp(s1[1])) - 48)) * 60) + (((int_cpp(s1[3]) - 48)) * 10)) + int_cpp(s1[4])) - 48);
  c = (((a + b)) / 2);
  k = (c / 60);
  l = (c % 60);
  if (((k / 10) == 0))
  {
    write("0", k, ":");
  } else
  {
    write(k, ":");
  }
  if (((l / 10) == 0))
  {
    write("0", l);
  } else
  {
    write(l);
  }
}
