// Translated from solution.cpp.

var maxn = (2e5 + 10);

var n: dynamic;

var cur: dynamic;

var keep: dynamic;

var basis: dynamic;

func ins(x: dynamic)
{
  var now = x;
  for (var i in basis)
  {
    x = min(x, (x ^ i));
  }
  if ((x > 0))
  {
    keep.push_back(now);
    basis.push_back(x);
    {
      var i = (cpp_cast(basis.size()) - 1);
      while ((i > 0))
      {
        if ((basis[i] > basis[(i - 1)]))
        {
          swap(basis[i], basis[(i - 1)]);
        } else
        {
          break;
        }
        i -= 1;
      }
    }
  }
}

func main()
{
  scanf("%d", (&n));
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d", (&s[i]));
      i += 1;
    }
  }
  sort(s.begin(), s.end());
  var has = 0;
  var pt = 0;
  {
    var i = 1;
    while ((i <= 19))
    {
      while (((pt < n) && (s[pt] < ((1 << i)))))
      {
        ins(s[cpp_update(pt, "++")]);
      }
      if ((cpp_cast(basis.size()) == i))
      {
        has = i;
      }
      i += 1;
    }
  }
  if ((has == 0))
  {
    return (!printf("0 \n0"));
  }
  printf("%d\n", has);
  cur = [0, 1];
  {
    var i = 1;
    while ((i < has))
    {
      {
        var j = (cpp_cast(cur.size()) - 1);
        while ((j >= 0))
        {
          cur.push_back(((cur[j] * 2) + 1));
          cur[j] <<= 1;
          j -= 1;
        }
      }
      i += 1;
    }
  }
  var toprint = 0;
  for (var i in cur)
  {
    toprint = 0;
    {
      var j = 0;
      while ((j < has))
      {
        if (((i >> j) & 1))
        {
          toprint ^= keep[j];
        }
        j += 1;
      }
    }
    printf("%d ", toprint);
  }
}
