// Translated from solution.cpp.

var maxn = (5e5 + 6);

var sp = cpp_array(maxn);

var divisor = cpp_array(maxn);

func seive()
{
  {
    var i = 2;
    while ((i < maxn))
    {
      sp[i] = i;
      i += 1;
    }
  }
  {
    var i = 2;
    while ((i < maxn))
    {
      {
        var j = 1;
        while ((((1 * i) * j) < maxn))
        {
          sp[(i * j)] = min(sp[(i * j)], sp[i]);
          divisor[(i * j)].push_back(i);
          j += 1;
        }
      }
      i += 1;
    }
  }
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  seive();
  var n: dynamic;
  var q: dynamic;
  read(n, q);
  var arr = cpp_construct((n + 1));
  {
    var i = 1;
    while ((i <= n))
    {
      read(arr[i]);
      i += 1;
    }
  }
  var in_self = cpp_construct((n + 1));
  var in_self_cnt = 0;
  var get = __cpp_lambda_1;
  var ans = 0;
  while (cpp_update(q, "--"))
  {
    var pos: dynamic;
    read(pos);
    var prime_factors: dynamic;
    var num = arr[pos];
    while ((num > 1))
    {
      var x = sp[num];
      prime_factors.push_back(x);
      while (((num % x) == 0))
      {
        num /= x;
      }
    }
    sort(prime_factors.begin(), prime_factors.end());
    num = arr[pos];
    if (in_self[pos])
    {
      for (var d in divisor[num])
      {
        divi[d] -= 1;
      }
      in_self[pos] = 0;
      in_self_cnt -= 1;
      ans -= (in_self_cnt - get(prime_factors));
    } else
    {
      ans += (in_self_cnt - get(prime_factors));
      for (var d in divisor[num])
      {
        divi[d] += 1;
      }
      in_self[pos] = 1;
      in_self_cnt += 1;
    }
    write(ans, "\n");
  }
}

func __cpp_lambda_1(vec: dynamic)
{
  var n = vec.size();
  var coprime = 0;
  {
    var mask = 0;
    while ((mask < ((1 << n))))
    {
      var d = 1;
      var bitCnt = 0;
      {
        var i = 0;
        while ((i < n))
        {
          if ((((mask >> i)) & 1))
          {
            d *= vec[i];
            bitCnt += 1;
          }
          i += 1;
        }
      }
      if ((bitCnt & 1))
      {
        coprime += divi[d];
      } else
      {
        coprime -= divi[d];
      }
      mask += 1;
    }
  }
  return coprime;
}
