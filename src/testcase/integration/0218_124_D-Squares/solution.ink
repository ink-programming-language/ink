// Translated from solution.cpp.

func f(s: dynamic)
{
  var k = 0;
  {
    var i = 0;
    while ((i < s.size()))
    {
      k = (((10 * k) + int_cpp(s[i])) - 48);
      i += 1;
    }
  }
  return k;
}

func main()
{
  var n: dynamic;
  var k: dynamic;
  var l: dynamic;
  var m: dynamic;
  var i: dynamic;
  var j: dynamic;
  var x3: dynamic;
  var y3: dynamic;
  var x4: dynamic;
  var y4: dynamic;
  var a: dynamic;
  var b: dynamic;
  var x1: dynamic;
  var x2: dynamic;
  var y1: dynamic;
  var y2: dynamic;
  read(a, b, x1, y1, x2, y2);
  if (((x1 + y1) >= 0))
  {
    x3 = (((x1 + y1)) / ((2 * a)));
  } else
  {
    x3 = ((((x1 + y1)) / ((2 * a))) - 1);
  }
  if (((x1 - y1) >= 0))
  {
    y3 = (((x1 - y1)) / ((2 * b)));
  } else
  {
    y3 = ((((x1 - y1)) / ((2 * b))) - 1);
  }
  if (((x2 + y2) >= 0))
  {
    x4 = (((x2 + y2)) / ((2 * a)));
  } else
  {
    x4 = ((((x2 + y2)) / ((2 * a))) - 1);
  }
  if (((x2 - y2) >= 0))
  {
    y4 = (((x2 - y2)) / ((2 * b)));
  } else
  {
    y4 = ((((x2 - y2)) / ((2 * b))) - 1);
  }
  k = max(abs((x3 - x4)), abs((y3 - y4)));
  write(k);
  return 0;
}
