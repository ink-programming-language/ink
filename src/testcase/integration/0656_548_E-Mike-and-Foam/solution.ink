// Translated from solution.cpp.

var maxn: dynamic = (5e5 + 6);

var sp: dynamic = cpp_array(maxn);

var divisor: dynamic = cpp_array(maxn);

func seive() -> dynamic
{
  {
    var i: dynamic = 2;
    while ((i < maxn))
    {
      sp[i] = i;
      i += 1;
    }
  }
  {
    var i: dynamic = 2;
    while ((i < maxn))
    {
      {
        var j: dynamic = 1;
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

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  seive();
  var n: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  read(n, q);
  var arr: dynamic = cpp_construct((n + 1));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(arr[i]);
      i += 1;
    }
  }
  var in_self: dynamic = cpp_construct((n + 1));
  var in_self_cnt: dynamic = 0;
  var get: dynamic = __cpp_lambda_1;
  var ans: dynamic = 0;
  while (cpp_update(q, "--"))
  {
    var pos: dynamic = cpp_uninitialized();
    read(pos);
    var prime_factors: dynamic = cpp_uninitialized();
    var num: dynamic = arr[pos];
    while ((num > 1))
    {
      var x: dynamic = sp[num];
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
      for (var d: dynamic in divisor[num])
      {
        divi[d] -= 1;
      }
      in_self[pos] = 0;
      in_self_cnt -= 1;
      ans -= (in_self_cnt - get(prime_factors));
    } else
    {
      ans += (in_self_cnt - get(prime_factors));
      for (var d: dynamic in divisor[num])
      {
        divi[d] += 1;
      }
      in_self[pos] = 1;
      in_self_cnt += 1;
    }
    write(ans, "\n");
  }
}

func __cpp_lambda_1(vec: dynamic) -> dynamic
{
  var n: dynamic = vec.size();
  var coprime: dynamic = 0;
  {
    var mask: dynamic = 0;
    while ((mask < ((1 << n))))
    {
      var d: dynamic = 1;
      var bitCnt: dynamic = 0;
      {
        var i: dynamic = 0;
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
