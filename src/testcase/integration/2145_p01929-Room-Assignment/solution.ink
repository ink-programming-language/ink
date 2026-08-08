// Translated from solution.cpp.

var mod = 1000000007;

class Mod
{
  var num: dynamic;
  func Mod()
  {
      cpp_base_construct(0);
    }
  func Mod(n: dynamic)
  {
      this->num = cpp_construct(((((n % mod) + mod)) % mod));
    }
  func Mod(n: dynamic)
  {
      cpp_base_construct(static_cast(n));
    }
  func cpp_function_1()
  {
      return num;
    }
}

func operator_add(a: dynamic, b: dynamic)
{
  return Mod((((a.num + b.num)) % mod));
}

func operator_add(a: dynamic, b: dynamic)
{
  return (Mod(a) + b);
}

func operator_add(a: dynamic, b: dynamic)
{
  return (b + a);
}

func operator(a: dynamic)
{
  return (a + Mod(1));
}

func operator_subtract(a: dynamic, b: dynamic)
{
  return Mod(((((mod + a.num) - b.num)) % mod));
}

func operator_subtract(a: dynamic, b: dynamic)
{
  return (Mod(a) - b);
}

func operator(a: dynamic)
{
  return (a - Mod(1));
}

func operator_multiply(a: dynamic, b: dynamic)
{
  return Mod((((cpp_cast(a.num) * b.num)) % mod));
}

func operator_multiply(a: dynamic, b: dynamic)
{
  return (Mod(a) * b);
}

func operator_multiply(a: dynamic, b: dynamic)
{
  return (Mod(b) * a);
}

func operator_multiply(a: dynamic, b: dynamic)
{
  return (Mod(b) * a);
}

func operator_add_assign(a: dynamic, b: dynamic)
{
  return cpp_assign(a, "=", (a + b));
}

func operator_add_assign(a: dynamic, b: dynamic)
{
  return cpp_assign(a, "=", (a + b));
}

func operator_subtract_assign(a: dynamic, b: dynamic)
{
  return cpp_assign(a, "=", (a - b));
}

func operator_subtract_assign(a: dynamic, b: dynamic)
{
  return cpp_assign(a, "=", (a - b));
}

func operator(a: dynamic, b: dynamic)
{
  return cpp_assign(a, "=", (a * b));
}

func operator(a: dynamic, b: dynamic)
{
  return cpp_assign(a, "=", (a * b));
}

func operator(a: dynamic, b: dynamic)
{
  return cpp_assign(a, "=", (a * b));
}

func operator(a: dynamic, n: dynamic)
{
  if ((n == 0))
  {
    return Mod(1);
  }
  var res = (((a * a)) ^ ((n / 2)));
  if ((n % 2))
  {
    res = (res * a);
  }
  return res;
}

func mod_pow(a: dynamic, n: dynamic)
{
  if ((n == 0))
  {
    return Mod(1);
  }
  var res = mod_pow(((a * a)), ((n / 2)));
  if ((n % 2))
  {
    res = (res * a);
  }
  return res;
}

func inv(a: dynamic)
{
  return (a ^ ((mod - 2)));
}

func operator_divide(a: dynamic, b: dynamic)
{
  assert((b.num != 0));
  return (a * inv(b));
}

func operator_divide(a: dynamic, b: dynamic)
{
  return (Mod(a) / b);
}

func operator(a: dynamic, b: dynamic)
{
  return cpp_assign(a, "=", (a / b));
}

var MAX_MOD_N = cpp_expression("#includ");

var fact = cpp_array(MAX_MOD_N);

var factinv = cpp_array(MAX_MOD_N);

func init(amax: dynamic = MAX_MOD_N)
{
  fact[0] = Mod(1);
  factinv[0] = 1;
  {
    var i = 0;
    while ((i < (amax - 1)))
    {
      fact[(i + 1)] = (fact[i] * Mod((i + 1)));
      factinv[(i + 1)] = (factinv[i] / Mod((i + 1)));
      i += 1;
    }
  }
}

func comb(a: dynamic, b: dynamic)
{
  return ((fact[a] * factinv[b]) * factinv[(a - b)]);
}

func dfs(rev_edges: dynamic, now: dynamic, used: dynamic)
{
  if (used[now])
  {
    assert(false);
  }
  used[now] = true;
  if ((rev_edges[now].size() == 0))
  {
    return 1;
  }
  assert((rev_edges[now].size() == 1));
  {
    return (1 + dfs(rev_edges, rev_edges[now][0], used));
  }
}

func main()
{
  init();
  while (true)
  {
    var N: dynamic;
    read(N);
    if ((!N))
    {
      break;
    }
    var edges: dynamic;
    {
      var i = 0;
      while ((i < N))
      {
        var a: dynamic;
        read(a);
        edges.push_back((a - 1));
        rev_edges[(a - 1)].push_back(i);
        i += 1;
      }
    }
    var ok = true;
    if (any_of(rev_edges.begin(), rev_edges.end(), __cpp_lambda_2))
    {
      ok = false;
    }
    var ans = 0;
    if (ok)
    {
      var loves = cpp_construct(N, -1);
      {
        var i = 0;
        while ((i < N))
        {
          if ((i == edges[edges[i]]))
          {
            loves[i] = edges[i];
          }
          i += 1;
        }
      }
      {
        var i = 0;
        while ((i < N))
        {
          if (((rev_edges[i].size() == 2) && (loves[i] == -1)))
          {
            ok = false;
          }
          i += 1;
        }
      }
      if (ok)
      {
        {
          var i = 0;
          while ((i < N))
          {
            if ((!used[i]))
            {
              if ((loves[i] != -1))
              {
                used[i] = true;
                if ((rev_edges[i].size() == 1))
                {
                  nums[i] = 1;
                } else
                {
                  for (var e in rev_edges[i])
                  {
                    if ((e != edges[i]))
                    {
                      nums[i] = (1 + dfs(rev_edges, e, used));
                    }
                  }
                }
              }
            }
            i += 1;
          }
        }
        if ((find_if(used.begin(), used.end(), __cpp_lambda_3) != used.end()))
        {
          ok = false;
        }
        if (ok)
        {
          ans = 1;
          var v: dynamic;
          {
            var i = 0;
            while ((i < N))
            {
              if ((loves[i] != -1))
              {
                if ((i < edges[i]))
                {
                  i += 1;
                  continue;
                }
                var num = (nums[i] + nums[edges[i]]);
                v.push_back(num);
              }
              i += 1;
            }
          }
          var two_num = count(v.begin(), v.end(), 2);
          var other_num = (v.size() - two_num);
          ans = fact[other_num];
          ans *= mod_pow(2, (two_num + other_num));
          ans *= fact[two_num];
          var kake = 0;
          {
            var t = 0;
            while ((t <= (two_num / 2)))
            {
              var rest = (two_num - (2 * t));
              if ((rest > (other_num + 1)))
              {
                t += 1;
                continue;
              } else
              {
                var plus = 0;
                plus = comb((other_num + t), t);
                plus *= comb((other_num + 1), rest);
                plus *= ((other_num + 1) + t);
                kake += plus;
              }
              t += 1;
            }
          }
          ans *= kake;
        }
      }
    }
    write(ans, "\n");
  }
  return 0;
}

func __cpp_lambda_2(v: dynamic)
{
  return (v.size() >= 3);
}

func __cpp_lambda_3(a: dynamic)
{
  return (a == false);
}
