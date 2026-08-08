// Translated from solution.cpp.

class node
{
  var next: dynamic = cpp_array(19);
}

var n: dynamic;

var q: dynamic;

var a = cpp_array(300005);

var nodes = cpp_array(300005);

func isReachable(curr: dynamic, end: dynamic)
{
  {
    var i = 0;
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

func main()
{
  scanf("%d %d", (&n), (&q));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  var ns = cpp_array(19, 19);
  var has = cpp_array(19);
  var wants = cpp_array(19);
  {
    var i = 1;
    while ((i <= n))
    {
      var hasCount = 0;
      var wantsCount = 0;
      {
        var bit = 0;
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
        var i2 = 0;
        while ((i2 < hasCount))
        {
          {
            var i3 = 0;
            while ((i3 < hasCount))
            {
              for (var v in ns[has[i2]][has[i3]])
              {
                if ((!nodes[v].next[has[i3]]))
                {
                  nodes[v].next[has[i3]] = i;
                  {
                    var bit = 0;
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
        var i2 = 0;
        while ((i2 < hasCount))
        {
          {
            var i3 = 0;
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
    var i = 0;
    while ((i < q))
    {
      var l: dynamic;
      var r: dynamic;
      scanf("%d %d", (&l), (&r));
      printf("%s\n", if (isReachable(l, r)) "Shi" else "Fou");
      i += 1;
    }
  }
  return 0;
}
