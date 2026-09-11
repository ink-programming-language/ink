// Translated from solution.cpp.

var absbs: dynamic = cpp_uninitialized();

var is: dynamic = cpp_array(3000);

var idx: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(32345);

var b: dynamic = cpp_array(32345);

func rec(bs: dynamic, nth: dynamic, r: dynamic) -> dynamic
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
  var n: dynamic = (is[idx[nth]] & (~bs));
  if ((n.count() == 1))
  {
    {
      var i: dynamic = 0;
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
    var rv: dynamic = rec((bs | is[idx[nth]]), (nth + 1), (r - 1));
    var o: dynamic = bs;
    var c: dynamic = 0;
    {
      var i: dynamic = 0;
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

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  var M: dynamic = cpp_uninitialized();
  var K: dynamic = cpp_uninitialized();
  read(N, M, K);
  var pop: dynamic = [];
  {
    var i: dynamic = 0;
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
  var r: dynamic = rec(BS(), 0, K);
  if ((r < 0))
  {
    write("Impossible", "\n");
  } else
  {
    write((K - r), "\n");
  }
}

func __cpp_lambda_1(a: dynamic, b: dynamic) -> dynamic
{
  return (pop[a] > pop[b]);
}
