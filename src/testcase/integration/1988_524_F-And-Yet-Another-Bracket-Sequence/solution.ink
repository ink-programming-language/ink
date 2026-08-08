// Translated from solution.cpp.

var N = (2e6 + 5);

var s = cpp_array(N);

var tp = cpp_array(N);

var rak = cpp_array(N);

var sa = cpp_array(N);

var tax = cpp_array(N);

var sl: dynamic;

var m: dynamic;

var cnt = cpp_array(N);

var lg = cpp_array(N);

var f = cpp_array(20, N);

var rr = cpp_array(N);

func radixSort()
{
  {
    var i = 1;
    while ((i <= m))
    {
      tax[i] = 0;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= sl))
    {
      tax[rak[i]] += 1;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      tax[i] += tax[(i - 1)];
      i += 1;
    }
  }
  {
    var i = sl;
    while (i)
    {
      sa[cpp_update(tax[rak[tp[i]]], "--")] = tp[i];
      i -= 1;
    }
  }
}

func build_sa()
{
  m = 125;
  {
    var i = 1;
    while ((i <= sl))
    {
      rak[i] = s[i];
      tp[i] = i;
      i += 1;
    }
  }
  radixSort();
  {
    var p = 0;
    var w = 1;
    while ((p < sl))
    {
      p = 0;
      {
        var i = 1;
        while ((i <= w))
        {
          tp[cpp_update(p, "++")] = ((sl - w) + i);
          i += 1;
        }
      }
      {
        var i = 1;
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
        var i = 2;
        while ((i <= sl))
        {
          rak[sa[i]] = if ((((tp[sa[i]] == tp[sa[(i - 1)]]) && (tp[(sa[i] + w)] == tp[(sa[(i - 1)] + w)])))) p else cpp_update(p, "++");
          i += 1;
        }
      }
      w <<= 1;
      m = p;
    }
  }
}

func make_st()
{
  var range = lg[sl];
  if ((cnt[sl] > 0))
  {
    {
      var i = 1;
      while ((i <= sl))
      {
        f[i][0] = cnt[i];
        i += 1;
      }
    }
    {
      var i = 1;
      while ((i <= range))
      {
        {
          var j = 1;
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
      var i = 1;
      while ((i <= sl))
      {
        f[i][0] = rr[i];
        i += 1;
      }
    }
    {
      var i = 1;
      while ((i <= range))
      {
        {
          var j = 1;
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

func query(l: dynamic, r: dynamic)
{
  var range = lg[((r - l) + 1)];
  if ((cnt[sl] > 0))
  {
    return min(f[l][range], f[((r - ((1 << range))) + 1)][range]);
  } else
  {
    return max(f[l][range], f[((r - ((1 << range))) + 1)][range]);
  }
}

func main()
{
  scanf("%s", (s + 1));
  sl = strlen((s + 1));
  {
    var i = 1;
    while ((i <= sl))
    {
      s[(i + sl)] = s[i];
      i += 1;
    }
  }
  sl <<= 1;
  build_sa();
  {
    var i = 1;
    while ((i <= sl))
    {
      cnt[i] = (cnt[(i - 1)] + (if (((s[i] == cpp_char("(")))) 1 else -1));
      i += 1;
    }
  }
  {
    var i = sl;
    while (i)
    {
      rr[i] = (rr[(i + 1)] + (if (((s[i] == cpp_char("(")))) 1 else -1));
      i -= 1;
    }
  }
  {
    var i = 2;
    while ((i <= sl))
    {
      lg[i] = (lg[(i - 1)] + ((i == ((1 << ((lg[(i - 1)] + 1)))))));
      i += 1;
    }
  }
  make_st();
  var l_cnt = 0;
  var r_cnt = 0;
  var ans_pos: dynamic;
  {
    var i = 1;
    while ((i <= sl))
    {
      var p = sa[i];
      if ((p > (sl / 2)))
      {
        i += 1;
        continue;
      }
      var res = query(p, ((p + (sl / 2)) - 1));
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
    var i = 1;
    while ((i <= l_cnt))
    {
      printf("(");
      i += 1;
    }
  }
  {
    var i = ans_pos;
    while ((i <= ((ans_pos + (sl / 2)) - 1)))
    {
      printf("%c", s[i]);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= r_cnt))
    {
      printf(")");
      i += 1;
    }
  }
  return 0;
}
