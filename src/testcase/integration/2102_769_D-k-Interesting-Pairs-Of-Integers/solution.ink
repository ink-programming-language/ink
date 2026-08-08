// Translated from solution.cpp.

var mxN = (1e4 + 2);

var adj = cpp_array(mxN);

func main()
{
  var n: dynamic;
  var a: dynamic;
  var b: dynamic;
  var m: dynamic;
  var i: dynamic;
  var j: dynamic;
  var cn = 0;
  var sm = 0;
  var mx = INT_MIN;
  var mn = INT_MAX;
  var k: dynamic;
  var s: dynamic;
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
      for (var j in adj[i])
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
