// Translated from solution.cpp.

func inversions(p: dynamic)
{
  var cnt = 0;
  var n = p.size();
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = (i + 1);
        while ((j < n))
        {
          if ((p[i] > p[j]))
          {
            cnt += 1;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  return cnt;
}

func solve(p: dynamic, k: dynamic)
{
  if ((k == 0))
  {
    return inversions(p);
  }
  var n = p.size();
  var ans = 0.0;
  {
    var i = 0;
    while ((i < n))
    {
      var prob = (2.0 / ((n * ((n + 1)))));
      {
        var j = i;
        while ((j < n))
        {
          var np = p;
          reverse((np.begin() + min(i, j)), ((np.begin() + max(i, j)) + 1));
          ans += (prob * solve(np, (k - 1)));
          j += 1;
        }
      }
      i += 1;
    }
  }
  return ans;
}

func main()
{
  var n: dynamic;
  var k: dynamic;
  read(n, k);
  for (var x in p)
  {
    read(x);
  }
  write(setprecision(10), fixed, solve(p, k), "\n");
  return 0;
}
