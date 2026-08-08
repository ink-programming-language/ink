// Translated from solution.cpp.

var B = 300;

func main()
{
  cin.tie(null);
  ios.sync_with_stdio(false);
  var n: dynamic;
  var m: dynamic;
  var q: dynamic;
  read(n, m, q);
  for (var ai in a)
  {
    read(ai);
  }
  {
    var i = 0;
    while ((i < m))
    {
      var k: dynamic;
      read(k);
      s[i].resize(k);
      for (var sij in s[i])
      {
        read(sij);
        sij -= 1;
        init_sum[i] += a[sij];
      }
      i += 1;
    }
  }
  var is_large = __cpp_lambda_1;
  var id: dynamic;
  var large_num = 0;
  {
    var i = 0;
    while ((i < m))
    {
      if (is_large(i))
      {
        for (var si in s[i])
        {
          b[si] = true;
        }
        {
          var j = 0;
          while ((j < m))
          {
            var cnt = 0;
            for (var sj in s[j])
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
    var op: dynamic;
    read(op);
    var k: dynamic;
    read(k);
    k -= 1;
    if ((op == cpp_char("?")))
    {
      if (is_large(k))
      {
        write((init_sum[k] + large_sum[k]), "\n");
      } else
      {
        var ans = init_sum[k];
        for (var sk in s[k])
        {
          ans += small_add[sk];
        }
        {
          var i = 0;
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
      var x: dynamic;
      read(x);
      {
        var i = 0;
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
        for (var sk in s[k])
        {
          small_add[sk] += x;
        }
      }
    }
  }
  return 0;
}

func __cpp_lambda_1(i: dynamic)
{
  return (s[i].size() >= B);
}
