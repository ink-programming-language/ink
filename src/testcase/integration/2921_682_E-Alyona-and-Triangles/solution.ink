// Translated from solution.cpp.

class tocka
{
  var x: dynamic;
  var y: dynamic;
  func operator_less(p: dynamic)
  {
      if ((x == p.x))
      {
        return (y < p.y);
      }
      return (x < p.x);
    }
}

var v: dynamic;

var gornji: dynamic;

var donji: dynamic;

var convex: dynamic;

func ccw(a: dynamic, b: dynamic, c: dynamic)
{
  return ((((a.x * ((b.y - c.y))) + (b.x * ((c.y - a.y)))) + (c.x * ((a.y - b.y)))));
}

func main()
{
  var n: dynamic;
  var s: dynamic;
  read(n, s);
  {
    var i = 0;
    while ((i < n))
    {
      var t: dynamic;
      scanf("%I64d%I64d", (&t.x), (&t.y));
      v.push_back(t);
      i += 1;
    }
  }
  sort(v.begin(), v.end());
  {
    var i = 0;
    while ((i < n))
    {
      while (((gornji.size() >= 2) && (ccw(gornji[(gornji.size() - 1)], gornji[(gornji.size() - 2)], v[i]) < 0)))
      {
        gornji.pop_back();
      }
      gornji.push_back(v[i]);
      i += 1;
    }
  }
  {
    var i = (n - 1);
    while ((i >= 0))
    {
      while (((donji.size() >= 2) && (ccw(donji[(donji.size() - 1)], donji[(donji.size() - 2)], v[i]) < 0)))
      {
        donji.pop_back();
      }
      donji.push_back(v[i]);
      i -= 1;
    }
  }
  {
    var i = 0;
    while ((i < (gornji.size() - 1)))
    {
      convex.push_back(gornji[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < (donji.size() - 1)))
    {
      convex.push_back(donji[i]);
      i += 1;
    }
  }
  var i = 0;
  var j = 1;
  var k = 2;
  var l = convex.size();
  {
    var x = 0;
    while ((x < (3 * n)))
    {
      while ((abs(ccw(convex[i], convex[j], convex[k])) < abs(ccw(convex[i], convex[j], convex[(((k + 1)) % l)]))))
      {
        k = (((k + 1)) % l);
      }
      while ((abs(ccw(convex[i], convex[j], convex[k])) < abs(ccw(convex[i], convex[(((j + 1)) % l)], convex[k]))))
      {
        j = (((j + 1)) % l);
      }
      while ((abs(ccw(convex[i], convex[j], convex[k])) < abs(ccw(convex[(((i + 1)) % l)], convex[j], convex[k]))))
      {
        i = (((i + 1)) % l);
      }
      x += 1;
    }
  }
  {
    var x = 0;
    while ((x < (3 * n)))
    {
      while ((abs(ccw(convex[i], convex[j], convex[k])) < abs(ccw(convex[i], convex[j], convex[((((k - 1) + l)) % l)]))))
      {
        k = ((((k - 1) + l)) % l);
      }
      while ((abs(ccw(convex[i], convex[j], convex[k])) < abs(ccw(convex[i], convex[((((j - 1) + l)) % l)], convex[k]))))
      {
        j = ((((j - 1) + l)) % l);
      }
      while ((abs(ccw(convex[i], convex[j], convex[k])) < abs(ccw(convex[((((i - 1) + l)) % l)], convex[j], convex[k]))))
      {
        i = ((((i - 1) + l)) % l);
      }
      x += 1;
    }
  }
  write((convex[i].x + ((convex[j].x - convex[k].x))), " ", (convex[i].y + ((convex[j].y - convex[k].y))), "\n");
  write((convex[j].x + ((convex[k].x - convex[i].x))), " ", (convex[j].y + ((convex[k].y - convex[i].y))), "\n");
  write((convex[k].x + ((convex[i].x - convex[j].x))), " ", (convex[k].y + ((convex[i].y - convex[j].y))), "\n");
  return 0;
}
