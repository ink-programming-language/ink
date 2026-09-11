// Translated from solution.cpp.

var mxN: dynamic = (1e4 + 2);

var adj: dynamic = cpp_array(mxN);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var cn: dynamic = 0;
  var sm: dynamic = 0;
  var mx: dynamic = INT_MIN;
  var mn: dynamic = INT_MAX;
  var k: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  read(n, m);
  {
    i = 0;
    while ((i <= 10000))
    {
      {
        j = i;
        while ((j <= 10000))
        {
          if ((builtin_popcount((i ^ j)) == m))
          {
            adj[i].push_back(j);
            if ((i != j))
            {
              adj[j].push_back(i);
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < n))
    {
      read(a);
      cnt[a] += 1;
      i += 1;
    }
  }
  {
    i = 0;
    while ((i <= 10000))
    {
      for (var j: dynamic in adj[i])
      {
        sm += (cnt[i] * cpp_cast(cnt[j]));
      }
      i += 1;
    }
  }
  if ((!m))
  {
    sm -= n;
  }
  write((sm / 2), "\n");
  return 0;
}
