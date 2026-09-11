// Translated from solution.cpp.

var B: dynamic = 300;

func main() -> dynamic
{
  cin.tie(null);
  ios.sync_with_stdio(false);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  read(n, m, q);
  for (var ai: dynamic in a)
  {
    read(ai);
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var k: dynamic = cpp_uninitialized();
      read(k);
      s[i].resize(k);
      for (var sij: dynamic in s[i])
      {
        read(sij);
        sij -= 1;
        init_sum[i] += a[sij];
      }
      i += 1;
    }
  }
  var is_large: dynamic = __cpp_lambda_1;
  var id: dynamic = cpp_uninitialized();
  var large_num: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      if (is_large(i))
      {
        for (var si: dynamic in s[i])
        {
          b[si] = true;
        }
        {
          var j: dynamic = 0;
          while ((j < m))
          {
            var cnt: dynamic = 0;
            for (var sj: dynamic in s[j])
            {
              cnt += b[sj];
            }
            ist_sz[j].emplace_back(cnt);
            j += 1;
          }
        }
        id.emplace_back(i);
        large_num += 1;
      }
      i += 1;
    }
  }
  while (cpp_update(q, "--"))
  {
    var op: dynamic = cpp_uninitialized();
    read(op);
    var k: dynamic = cpp_uninitialized();
    read(k);
    k -= 1;
    if ((op == cpp_char("?")))
    {
      if (is_large(k))
      {
        write((init_sum[k] + large_sum[k]), "\n");
      } else
      {
        var ans: dynamic = init_sum[k];
        for (var sk: dynamic in s[k])
        {
          ans += small_add[sk];
        }
        {
          var i: dynamic = 0;
          while ((i < large_num))
          {
            ans += (large_add[id[i]] * ist_sz[k][i]);
            i += 1;
          }
        }
        write(ans, "\n");
      }
    }
    if ((op == cpp_char("+")))
    {
      var x: dynamic = cpp_uninitialized();
      read(x);
      {
        var i: dynamic = 0;
        while ((i < large_num))
        {
          large_sum[id[i]] += (x * ist_sz[k][i]);
          i += 1;
        }
      }
      if (is_large(k))
      {
        large_add[k] += x;
      } else
      {
        for (var sk: dynamic in s[k])
        {
          small_add[sk] += x;
        }
      }
    }
  }
  return 0;
}

func __cpp_lambda_1(i: dynamic) -> dynamic
{
  return (s[i].size() >= B);
}
