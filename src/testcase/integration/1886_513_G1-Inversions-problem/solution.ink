// Translated from solution.cpp.

func inversions(p: dynamic) -> dynamic
{
  var cnt: dynamic = 0;
  var n: dynamic = p.size();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = (i + 1);
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

func solve(p: dynamic, k: dynamic) -> dynamic
{
  if ((k == 0))
  {
    return inversions(p);
  }
  var n: dynamic = p.size();
  var ans: dynamic = 0.0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var prob: dynamic = (2.0 / ((n * ((n + 1)))));
      {
        var j: dynamic = i;
        while ((j < n))
        {
          var np: dynamic = p;
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

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, k);
  for (var x: dynamic in p)
  {
    read(x);
  }
  write(setprecision(10), fixed, solve(p, k), "\n");
  return 0;
}
