// Translated from solution.cpp.

var a = cpp_array(4000001);

var b = cpp_array(4000001);

var qsum = cpp_array(4000001);

var rakha = cpp_array(4000001);

class node
{
  var x: dynamic;
  var y: dynamic;
  var sum: dynamic;
}

class cmp
{
  func operator_call(a: dynamic, b: dynamic)
  {
      return (a.sum < b.sum);
    }
}

var pq: dynamic;

var ar = cpp_array(100, 100);

func main()
{
  var n: dynamic;
  var t: dynamic;
  var i: dynamic;
  var j: dynamic;
  var n1: dynamic;
  var k: dynamic;
  var ans: dynamic;
  var m: dynamic;
  var f = 0;
  var s = 0;
  var c = 0;
  var p = 1;
  var c1 = 0;
  var p1 = 0;
  var p2 = 0;
  var res = 0;
  var c2: dynamic;
  var s1: dynamic;
  var s2: dynamic;
  var ss: dynamic;
  var pp: dynamic;
  var x: dynamic;
  var y: dynamic;
  var st: dynamic;
  var str: dynamic;
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
