// Translated from solution.cpp.

var maxn: dynamic = (2e5 + 5);

var Mod: dynamic = (1e9 + 7);

func powmod(a: dynamic, b: dynamic) -> dynamic
{
  var res: dynamic = 1;
  a %= Mod;
  assert((b >= 0));
  {
    while (b)
    {
      if ((b & 1))
      {
        res = ((res * a) % Mod);
      }
      a = ((a * a) % Mod);
      b >>= 1;
    }
  }
  return res;
}

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  return  (b) ? gcd(b, (a % b)) : a;
}

var T: dynamic = cpp_uninitialized();

var num: dynamic = cpp_array(4);

var sum: dynamic = cpp_uninitialized();

var arr: dynamic = cpp_array(maxn);

func solve(kai: dynamic) -> dynamic
{
  var so_num: dynamic = [0];
  {
    var i: dynamic = 0;
    while ((i <= 3))
    {
      so_num[i] = num[i];
      i += 1;
    }
  }
  var kk: dynamic = kai;
  var boo: dynamic = true;
  {
    var i: dynamic = 1;
    while ((i <= sum))
    {
      arr[i] = kk;
      so_num[kk] -= 1;
      if (((!boo) || (i == sum)))
      {
        break;
      }
      var __cpp_switch_1: dynamic = kk;
      if (__cpp_switch_1 == 0)
      {
        if ((so_num[1] <= 0))
        {
        boo = false;
        }
        kk = 1;
        break;
      }
      else if (__cpp_switch_1 == 1)
      {
        if (so_num[0])
        {
        kk = 0;
        break;
        }
        if ((!so_num[2]))
        {
        boo = false;
        }
        kk = 2;
        break;
      }
      else if (__cpp_switch_1 == 2)
      {
        if (so_num[3])
        {
        kk = 3;
        break;
        }
        if ((!so_num[1]))
        {
        boo = false;
        }
        kk = 1;
        break;
      }
      else if (__cpp_switch_1 == 3)
      {
        if ((so_num[2] <= 0))
        {
        boo = false;
        }
        kk = 2;
        break;
      }
      i += 1;
    }
  }
  if ((!boo))
  {
    return false;
  }
  return true;
}

func main() -> dynamic
{
  var T: dynamic = 1;
  {
    var cas: dynamic = 1;
    while ((cas <= T))
    {
      sum = 0;
      {
        var i: dynamic = 0;
        while ((i <= 3))
        {
          scanf("%d", (&num[i]));
          i += 1;
        }
      }
      {
        var i: dynamic = 0;
        while ((i <= 3))
        {
          sum += num[i];
          i += 1;
        }
      }
      var bo: dynamic = false;
      {
        var j: dynamic = 0;
        while ((j <= 3))
        {
          if (((num[j] != 0) && solve(j)))
          {
            bo = true;
            break;
          }
          j += 1;
        }
      }
      if ((!bo))
      {
        puts("NO");
      } else
      {
        puts("YES");
        {
          var i: dynamic = 1;
          while ((i <= sum))
          {
            printf("%d ", arr[i]);
            i += 1;
          }
        }
        puts("");
      }
      cas += 1;
    }
  }
  return 0;
}
