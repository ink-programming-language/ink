// Translated from solution.cpp.

var N: dynamic = (2e6 + 5);

var s: dynamic = cpp_array(N);

var tp: dynamic = cpp_array(N);

var rak: dynamic = cpp_array(N);

var sa: dynamic = cpp_array(N);

var tax: dynamic = cpp_array(N);

var sl: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var cnt: dynamic = cpp_array(N);

var lg: dynamic = cpp_array(N);

var f: dynamic = cpp_array(20, N);

var rr: dynamic = cpp_array(N);

func radixSort() -> dynamic
{
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      tax[i] = 0;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= sl))
    {
      tax[rak[i]] += 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      tax[i] += tax[(i - 1)];
      i += 1;
    }
  }
  {
    var i: dynamic = sl;
    while (i)
    {
      sa[cpp_update(tax[rak[tp[i]]], "--")] = tp[i];
      i -= 1;
    }
  }
}

func build_sa() -> dynamic
{
  m = 125;
  {
    var i: dynamic = 1;
    while ((i <= sl))
    {
      rak[i] = s[i];
      tp[i] = i;
      i += 1;
    }
  }
  radixSort();
  {
    var p: dynamic = 0;
    var w: dynamic = 1;
    while ((p < sl))
    {
      p = 0;
      {
        var i: dynamic = 1;
        while ((i <= w))
        {
          tp[cpp_update(p, "++")] = ((sl - w) + i);
          i += 1;
        }
      }
      {
        var i: dynamic = 1;
        while ((i <= sl))
        {
          if ((sa[i] > w))
          {
            tp[cpp_update(p, "++")] = (sa[i] - w);
          }
          i += 1;
        }
      }
      radixSort();
      swap(tp, rak);
      rak[sa[1]] = cpp_assign(p, "=", 1);
      {
        var i: dynamic = 2;
        while ((i <= sl))
        {
          rak[sa[i]] =  ((((tp[sa[i]] == tp[sa[(i - 1)]]) && (tp[(sa[i] + w)] == tp[(sa[(i - 1)] + w)])))) ? p : cpp_update(p, "++");
          i += 1;
        }
      }
      w <<= 1;
      m = p;
    }
  }
}

func make_st() -> dynamic
{
  var range: dynamic = lg[sl];
  if ((cnt[sl] > 0))
  {
    {
      var i: dynamic = 1;
      while ((i <= sl))
      {
        f[i][0] = cnt[i];
        i += 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i <= range))
      {
        {
          var j: dynamic = 1;
          while ((((j + ((1 << i))) - 1) <= sl))
          {
            f[j][i] = min(f[j][(i - 1)], f[(j + ((1 << ((i - 1)))))][(i - 1)]);
            j += 1;
          }
        }
        i += 1;
      }
    }
  } else
  {
    {
      var i: dynamic = 1;
      while ((i <= sl))
      {
        f[i][0] = rr[i];
        i += 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i <= range))
      {
        {
          var j: dynamic = 1;
          while ((((j + ((1 << i))) - 1) <= sl))
          {
            f[j][i] = max(f[j][(i - 1)], f[(j + ((1 << ((i - 1)))))][(i - 1)]);
            j += 1;
          }
        }
        i += 1;
      }
    }
  }
}

func query(l: dynamic, r: dynamic) -> dynamic
{
  var range: dynamic = lg[((r - l) + 1)];
  if ((cnt[sl] > 0))
  {
    return min(f[l][range], f[((r - ((1 << range))) + 1)][range]);
  } else
  {
    return max(f[l][range], f[((r - ((1 << range))) + 1)][range]);
  }
}

func main() -> dynamic
{
  scanf("%s", (s + 1));
  sl = strlen((s + 1));
  {
    var i: dynamic = 1;
    while ((i <= sl))
    {
      s[(i + sl)] = s[i];
      i += 1;
    }
  }
  sl <<= 1;
  build_sa();
  {
    var i: dynamic = 1;
    while ((i <= sl))
    {
      cnt[i] = (cnt[(i - 1)] + ( (((s[i] == cpp_char("(")))) ? 1 : -1));
      i += 1;
    }
  }
  {
    var i: dynamic = sl;
    while (i)
    {
      rr[i] = (rr[(i + 1)] + ( (((s[i] == cpp_char("(")))) ? 1 : -1));
      i -= 1;
    }
  }
  {
    var i: dynamic = 2;
    while ((i <= sl))
    {
      lg[i] = (lg[(i - 1)] + ((i == ((1 << ((lg[(i - 1)] + 1)))))));
      i += 1;
    }
  }
  make_st();
  var l_cnt: dynamic = 0;
  var r_cnt: dynamic = 0;
  var ans_pos: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
    while ((i <= sl))
    {
      var p: dynamic = sa[i];
      if ((p > (sl / 2)))
      {
        i += 1;
        continue;
      }
      var res: dynamic = query(p, ((p + (sl / 2)) - 1));
      if ((cnt[sl] > 0))
      {
        if (((res - cnt[(p - 1)]) < 0))
        {
          i += 1;
          continue;
        }
        r_cnt = abs((cnt[sl] / 2));
        ans_pos = p;
        break;
      } else
      {
        if (((res - rr[(p + (sl / 2))]) > 0))
        {
          i += 1;
          continue;
        }
        l_cnt = abs((cnt[sl] / 2));
        ans_pos = p;
        break;
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= l_cnt))
    {
      printf("(");
      i += 1;
    }
  }
  {
    var i: dynamic = ans_pos;
    while ((i <= ((ans_pos + (sl / 2)) - 1)))
    {
      printf("%c", s[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= r_cnt))
    {
      printf(")");
      i += 1;
    }
  }
  return 0;
}
