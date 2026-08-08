// Translated from solution.cpp.

func smin(a: dynamic, b: dynamic)
{
  if ((a > b))
  {
    a = b;
  }
}

func smax(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    a = b;
  }
}

func power(a: dynamic, b: dynamic, m: dynamic, ans: dynamic = 1)
{
  {
    while (b)
    {
      if ((b & 1))
      {
        ans = (((1 * ans) * a) % m);
      }
      b >>= 1;
      a = (((1 * a) * a) % m);
    }
  }
  return ans;
}

class point
{
  var first: dynamic;
  var second: dynamic;
  func point(first: dynamic = 0, second: dynamic = 0)
  {
      this->first = cpp_construct(first);
      this->second = cpp_construct(second);
    }
  func operator_subtract(a: dynamic)
  {
      return point((first - a.first), (second - a.second));
    }
  func operator(a: dynamic)
  {
      return ((cpp_cast(first) * a.second) - (cpp_cast(second) * a.first));
    }
  func operator_multiply(a: dynamic)
  {
      return ((cpp_cast(first) * a.first) + (cpp_cast(second) * a.second));
    }
  func input()
  {
      scanf("%d%d", (&first), (&second));
    }
}

func sgn(a: dynamic)
{
  return (((a > 0)) - ((a < 0)));
}

func is_on(a: dynamic, b: dynamic, c: dynamic)
{
  if (sgn((((a - b)) ^ ((a - c)))))
  {
    return 0;
  }
  return (sgn((((a - c)) * ((c - b)))) > 0);
}

var ston = cpp_array(1110);

var mon = cpp_array(1110);

var id = cpp_array(10);

var vst = cpp_array(1110);

var runs: dynamic;

var used: dynamic;

var k: dynamic;

var n: dynamic;

var adj = cpp_array(1110, 10);

func judge(u: dynamic)
{
  vst[u] = runs;
  if ((used >= k))
  {
    return false;
  }
  var tmp = id[cpp_update(used, "++")];
  {
    var i = 0;
    while ((i < adj[tmp][u].size()))
    {
      var v = adj[tmp][u][i];
      if ((vst[v] != runs))
      {
        if ((!judge(v)))
        {
          return false;
        }
      }
      i += 1;
    }
  }
  return true;
}

func main()
{
  scanf("%d%d", (&k), (&n));
  {
    var i = 0;
    while ((i < k))
    {
      ston[i].input();
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      mon[i].input();
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < k))
    {
      {
        var p = 0;
        while ((p < n))
        {
          {
            var j = 0;
            while ((j < n))
            {
              if ((j != p))
              {
                if (is_on(ston[i], mon[p], mon[j]))
                {
                  adj[i][p].push_back(j);
                }
                if ((adj[i][p].size() >= k))
                {
                  break;
                }
              }
              j += 1;
            }
          }
          p += 1;
        }
      }
      i += 1;
    }
  }
  var res = 0;
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < k))
        {
          id[j] = j;
          j += 1;
        }
      }
      while (true)
      {
        runs += 1;
        used = 0;
        if (judge(i))
        {
          res += 1;
          break;
        }
        if (!((next_permutation(id, (id + k)))))
        {
          break;
        }
      }
      i += 1;
    }
  }
  printf("%d\n", res);
  return 0;
}
