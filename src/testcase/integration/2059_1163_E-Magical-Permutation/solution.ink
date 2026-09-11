// Translated from solution.cpp.

var maxn: dynamic = (2e5 + 10);

var n: dynamic = cpp_uninitialized();

var cur: dynamic = cpp_uninitialized();

var keep: dynamic = cpp_uninitialized();

var basis: dynamic = cpp_uninitialized();

func ins(x: dynamic) -> dynamic
{
  var now: dynamic = x;
  for (var i: dynamic in basis)
  {
    x = min(x, (x ^ i));
  }
  if ((x > 0))
  {
    keep.push_back(now);
    basis.push_back(x);
    {
      var i: dynamic = (cpp_cast(basis.size()) - 1);
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

func main() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%d", (&s[i]));
      i += 1;
    }
  }
  sort(s.begin(), s.end());
  var has: dynamic = 0;
  var pt: dynamic = 0;
  {
    var i: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i < has))
    {
      {
        var j: dynamic = (cpp_cast(cur.size()) - 1);
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
  var toprint: dynamic = 0;
  for (var i: dynamic in cur)
  {
    toprint = 0;
    {
      var j: dynamic = 0;
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
