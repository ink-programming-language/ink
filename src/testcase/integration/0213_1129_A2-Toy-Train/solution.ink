// Translated from solution.cpp.

var v = cpp_array(5005);

var n: dynamic;

var m: dynamic;

var x: dynamic;

var dp = cpp_array(5005);

func dist(a: dynamic, b: dynamic)
{
  if ((b >= a))
  {
    return (b - a);
  } else
  {
    return ((b - a) + n);
  }
}

func cmp(xx: dynamic, yy: dynamic)
{
  return (dist(x, xx) < dist(x, yy));
}

func main()
{
  read(n, m);
  {
    var i = 0;
    while ((i < m))
    {
      var a: dynamic;
      var b: dynamic;
      read(a, b);
      a -= 1;
      b -= 1;
      v[a].push_back(b);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      if (v[i].empty())
      {
        i += 1;
        continue;
      }
      x = i;
      sort(v[i].begin(), v[i].end(), cmp);
      dp[i] = ((n * cpp_cast(((v[i].size() - 1)))) + dist(i, v[i][0]));
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      var ma = -1e9;
      {
        var j = 0;
        while ((j < n))
        {
          if (dp[j])
          {
            ma = max((dist(i, j) + dp[j]), ma);
          }
          j += 1;
        }
      }
      write(ma, " ");
      i += 1;
    }
  }
}
