// Translated from solution.cpp.

var a: dynamic;

var b: dynamic;

var c: dynamic;

var d: dynamic;

var i: dynamic;

var e: dynamic;

var f: dynamic;

var g: dynamic;

var n: dynamic;

var m: dynamic;

var k: dynamic;

var l: dynamic;

var s: dynamic;

var s1: dynamic;

func main()
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
