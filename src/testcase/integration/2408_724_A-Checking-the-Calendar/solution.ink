// Translated from solution.cpp.

var a: dynamic = cpp_array(4000001);

var b: dynamic = cpp_array(4000001);

var qsum: dynamic = cpp_array(4000001);

var rakha: dynamic = cpp_array(4000001);

class node
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var sum: dynamic = cpp_uninitialized();
}

class cmp
{
  func operator_call(a: dynamic, b: dynamic) -> dynamic
  {
      return (a.sum < b.sum);
    }
}

var pq: dynamic = cpp_uninitialized();

var ar: dynamic = cpp_array(100, 100);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var n1: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var ans: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var f: dynamic = 0;
  var s: dynamic = 0;
  var c: dynamic = 0;
  var p: dynamic = 1;
  var c1: dynamic = 0;
  var p1: dynamic = 0;
  var p2: dynamic = 0;
  var res: dynamic = 0;
  var c2: dynamic = cpp_uninitialized();
  var s1: dynamic = cpp_uninitialized();
  var s2: dynamic = cpp_uninitialized();
  var ss: dynamic = cpp_uninitialized();
  var pp: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var st: dynamic = cpp_uninitialized();
  var str: dynamic = cpp_uninitialized();
  read(st, str);
  if ((st == str))
  {
    write("YES", "\n");
  } else
  {
    if (((st == "sunday") && (((str == "tuesday") || (str == "wednesday")))))
    {
      write("YES", "\n");
    } else if (((st == "tuesday") && (((str == "thursday") || (str == "friday")))))
    {
      write("YES", "\n");
    } else if (((st == "saturday") && (((str == "monday") || (str == "tuesday")))))
    {
      write("YES", "\n");
    } else if (((st == "monday") && (((str == "wednesday") || (str == "thursday")))))
    {
      write("YES", "\n");
    } else if (((st == "wednesday") && (((str == "friday") || (str == "saturday")))))
    {
      write("YES", "\n");
    } else if (((st == "thursday") && (((str == "sunday") || (str == "saturday")))))
    {
      write("YES", "\n");
    } else if (((st == "friday") && (((str == "sunday") || (str == "monday")))))
    {
      write("YES", "\n");
    } else
    {
      write("NO", "\n");
    }
  }
  return 0;
}
