// Translated from solution.cpp.

class node
{
  var next: dynamic = cpp_array(19);
}

var n: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(300005);

var nodes: dynamic = cpp_array(300005);

func isReachable(curr: dynamic, end: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < 19))
    {
      if (((((a[end] & ((1 << i)))) && nodes[curr].next[i]) && (nodes[curr].next[i] <= end)))
      {
        return true;
      }
      i += 1;
    }
  }
  return false;
}

func main() -> dynamic
{
  scanf("%d %d", (&n), (&q));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  var ns: dynamic = cpp_array(19, 19);
  var has: dynamic = cpp_array(19);
  var wants: dynamic = cpp_array(19);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var hasCount: dynamic = 0;
      var wantsCount: dynamic = 0;
      {
        var bit: dynamic = 0;
        while ((bit < 19))
        {
          if (((a[i] & ((1 << bit)))))
          {
            has[cpp_update(hasCount, "++")] = bit;
            nodes[i].next[bit] = i;
          } else
          {
            wants[cpp_update(wantsCount, "++")] = bit;
          }
          bit += 1;
        }
      }
      {
        var i2: dynamic = 0;
        while ((i2 < hasCount))
        {
          {
            var i3: dynamic = 0;
            while ((i3 < hasCount))
            {
              for (var v: dynamic in ns[has[i2]][has[i3]])
              {
                if ((!nodes[v].next[has[i3]]))
                {
                  nodes[v].next[has[i3]] = i;
                  {
                    var bit: dynamic = 0;
                    while ((bit < 19))
                    {
                      if ((!nodes[v].next[i]))
                      {
                        ns[has[i3]][bit].push_back(v);
                      }
                      bit += 1;
                    }
                  }
                }
              }
              ns[has[i2]][has[i3]].clear();
              i3 += 1;
            }
          }
          i2 += 1;
        }
      }
      {
        var i2: dynamic = 0;
        while ((i2 < hasCount))
        {
          {
            var i3: dynamic = 0;
            while ((i3 < wantsCount))
            {
              ns[has[i2]][wants[i3]].push_back(i);
              i3 += 1;
            }
          }
          i2 += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < q))
    {
      var l: dynamic = cpp_uninitialized();
      var r: dynamic = cpp_uninitialized();
      scanf("%d %d", (&l), (&r));
      printf("%s\n",  (isReachable(l, r)) ? "Shi" : "Fou");
      i += 1;
    }
  }
  return 0;
}
