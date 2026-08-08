// Translated from solution.cpp.

var absbs: dynamic;

var is = cpp_array(3000);

var idx: dynamic;

var a = cpp_array(32345);

var b = cpp_array(32345);

func rec(bs: dynamic, nth: dynamic, r: dynamic)
{
  if ((r < 0))
  {
    return r;
  }
  if ((bs == absbs))
  {
    return r;
  }
  while (((is[idx[nth]] & (~bs))).none())
  {
    nth += 1;
  }
  var n = (is[idx[nth]] & (~bs));
  if ((n.count() == 1))
  {
    {
      var i = 0;
      while (true)
      {
        if (n[i])
        {
          return rec((bs | is[((a[i] ^ b[i]) ^ idx[nth])]), (nth + 1), (r - 1));
        }
        i += 1;
      }
    }
  } else
  {
    var rv = rec((bs | is[idx[nth]]), (nth + 1), (r - 1));
    var o = bs;
    var c = 0;
    {
      var i = 0;
      while (n.any())
      {
        if (n[i])
        {
          o |= (bs | is[((a[i] ^ b[i]) ^ idx[nth])]);
          c += 1;
          n[i] = false;
        }
        i += 1;
      }
    }
    return max(rv, rec(o, (nth + 1), (r - c)));
  }
}

func main()
{
  var N: dynamic;
  var M: dynamic;
  var K: dynamic;
  read(N, M, K);
  var pop = [];
  {
    var i = 0;
    while ((i < M))
    {
      absbs[i] = true;
      read(a[i], b[i]);
      a[i] -= 1;
      b[i] -= 1;
      pop[a[i]] += 1;
      pop[b[i]] += 1;
      is[a[i]][i] = true;
      is[b[i]][i] = true;
      i += 1;
    }
  }
  idx.resize(N);
  iota(begin(idx), end(idx), 0);
  sort(begin(idx), end(idx), __cpp_lambda_1);
  var r = rec(BS(), 0, K);
  if ((r < 0))
  {
    write("Impossible", "\n");
  } else
  {
    write((K - r), "\n");
  }
}

func __cpp_lambda_1(a: dynamic, b: dynamic)
{
  return (pop[a] > pop[b]);
}
