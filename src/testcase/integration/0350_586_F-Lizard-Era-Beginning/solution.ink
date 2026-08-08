// Translated from solution.cpp.

var MOD = (1000000000 + 7);

var MAXN = (100000 + 100);

var MAGIC = 123123123;

var PI = (4 * atan(1.0));

var EPS = 1E-7;

class cmp_for_set
{
  func operator_call(a: dynamic, b: dynamic)
  {
      return (a > b);
    }
}

func time_elapsed()
{
  write("\nTIME ELAPSED: ", (cpp_cast(clock()) / CLOCKS_PER_SEC), " sec\n");
}

func gcd(a: dynamic, b: dynamic)
{
  return (if (((!b))) a else gcd(b, (a % b)));
}

func gcd(a: dynamic, b: dynamic, x: dynamic, y: dynamic)
{
  if ((!a))
  {
    x = 0;
    y = 1;
    return b;
  }
  var x1: dynamic;
  var y1: dynamic;
  var d = gcd((b % a), a, x1, y1);
  x = (y1 - (((b / a)) * x1));
  y = x1;
  return d;
}

func lcm(a: dynamic, b: dynamic)
{
  return (((a / gcd(a, b))) * b);
}

func neg_mod(a: dynamic, mod: dynamic)
{
  return (((((a % mod)) + mod)) % mod);
}

func binpow(x: dynamic, p: dynamic)
{
  var res = 1;
  while (p)
  {
    if ((p & 1))
    {
      res *= x;
    }
    x *= x;
    p >>= 1;
  }
  return res;
}

func binpow_mod(x: dynamic, p: dynamic, m: dynamic)
{
  var res = 1;
  while (p)
  {
    if ((p & 1))
    {
      res = (((res * x)) % m);
    }
    x = (((x * x)) % m);
    p >>= 1;
  }
  return res;
}

class state
{
  var mask: dynamic;
  var sum: dynamic = cpp_array(3);
  func state()
  {
      mask = 0;
      sum[0] = cpp_assign(sum[1], "=", cpp_assign(sum[2], "=", 0));
    }
}

func operator_less(a: dynamic, b: dynamic)
{
  return ((((a.sum[0] < b.sum[0]) || (((a.sum[0] == b.sum[0]) && (a.sum[1] < b.sum[1])))) || ((((a.sum[0] == b.sum[0]) && (a.sum[1] == b.sum[1])) && (a.sum[2] < b.sum[2])))));
}

var let_cpp = [cpp_char("L"), cpp_char("M"), cpp_char("W")];

func main()
{
  var n: dynamic;
  read(n);
  var vec1 = cpp_construct((n / 2), vector(3));
  var vec2 = cpp_construct((n - (n / 2)), vector(3));
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < 3))
        {
          if ((i < (n / 2)))
          {
            scanf("%I64d", (&vec1[i][j]));
          } else
          {
            scanf("%I64d", (&vec2[(i - (n / 2))][j]));
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var mem: dynamic;
  var pow3 = cpp_construct(20);
  pow3[0] = 1;
  {
    var i = 1;
    while ((i < 20))
    {
      pow3[i] = (pow3[(i - 1)] * 3);
      i += 1;
    }
  }
  var cmask = cpp_construct(20);
  {
    var mask = 0;
    while ((mask < pow3[vec1.size()]))
    {
      var mm = mask;
      {
        var j = 0;
        while ((j < vec1.size()))
        {
          cmask[j] = (mm % 3);
          mm /= 3;
          j += 1;
        }
      }
      var cur_state: dynamic;
      {
        var j = 0;
        while ((j < vec1.size()))
        {
          {
            var k = 0;
            while ((k < 3))
            {
              cur_state.sum[k] += vec1[j][k];
              k += 1;
            }
          }
          cur_state.sum[cmask[j]] -= vec1[j][cmask[j]];
          j += 1;
        }
      }
      cur_state.mask = mask;
      var cur_delta = make_pair((cur_state.sum[1] - cur_state.sum[0]), (cur_state.sum[2] - cur_state.sum[0]));
      if ((!mem.count(cur_delta)))
      {
        mem[cur_delta] = cur_state;
      } else if ((mem[cur_delta] < cur_state))
      {
        mem[cur_delta] = cur_state;
      }
      mask += 1;
    }
  }
  var best: dynamic;
  var best_sum = -9999999999999999;
  {
    var mask = 0;
    while ((mask < pow3[vec2.size()]))
    {
      var mm = mask;
      {
        var j = 0;
        while ((j < vec2.size()))
        {
          cmask[j] = (mm % 3);
          mm /= 3;
          j += 1;
        }
      }
      var cur_state: dynamic;
      {
        var j = 0;
        while ((j < vec2.size()))
        {
          {
            var k = 0;
            while ((k < 3))
            {
              cur_state.sum[k] += vec2[j][k];
              k += 1;
            }
          }
          cur_state.sum[cmask[j]] -= vec2[j][cmask[j]];
          j += 1;
        }
      }
      cur_state.mask = mask;
      var cur_delta = make_pair((cur_state.sum[1] - cur_state.sum[0]), (cur_state.sum[2] - cur_state.sum[0]));
      var need = cur_delta;
      need.first *= -1;
      need.second *= -1;
      if (mem.count(need))
      {
        var ss = mem[need];
        if (((cur_state.sum[0] + ss.sum[0]) > best_sum))
        {
          best_sum = (cur_state.sum[0] + ss.sum[0]);
          best = make_pair(ss.mask, cur_state.mask);
        }
      }
      mask += 1;
    }
  }
  if ((best_sum == -9999999999999999))
  {
    puts("Impossible");
  } else
  {
    {
      var j = 0;
      while ((j < vec1.size()))
      {
        var mm = best.first;
        {
          var j = 0;
          while ((j < vec1.size()))
          {
            cmask[j] = (mm % 3);
            mm /= 3;
            j += 1;
          }
        }
        {
          var k = 0;
          while ((k < 3))
          {
            if ((cmask[j] != k))
            {
              printf("%c", let_cpp[k]);
            }
            k += 1;
          }
        }
        printf("\n");
        j += 1;
      }
    }
    {
      var j = 0;
      while ((j < vec2.size()))
      {
        var mm = best.second;
        {
          var j = 0;
          while ((j < vec2.size()))
          {
            cmask[j] = (mm % 3);
            mm /= 3;
            j += 1;
          }
        }
        {
          var k = 0;
          while ((k < 3))
          {
            if ((cmask[j] != k))
            {
              printf("%c", let_cpp[k]);
            }
            k += 1;
          }
        }
        printf("\n");
        j += 1;
      }
    }
  }
  return 0;
}
