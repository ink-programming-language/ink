// Translated from solution.cpp.

var kNmax: dynamic = (2e5 + 10);

var kMod: dynamic = 998244353;

var n: dynamic = cpp_uninitialized();

var p: dynamic = cpp_array(kNmax);

var emptyLeft: dynamic = cpp_array(kNmax);

var emptyRight: dynamic = cpp_array(kNmax);

var countBigger: dynamic = cpp_array(kNmax);

var countSmaller: dynamic = cpp_array(kNmax);

var res: dynamic = cpp_uninitialized();

var seen: dynamic = cpp_array(kNmax);

class FT
{
  func FT(n: dynamic) -> dynamic
  {
      sz = n;
      arr.resize((sz + 5));
    }
  func lsb(x: dynamic) -> dynamic
  {
      return (((x & ((x - 1)))) ^ x);
    }
  func update(pos: dynamic) -> dynamic
  {
      {
        while ((pos <= sz))
        {
          arr[pos] += 1;
          pos += lsb(pos);
        }
      }
    }
  func query(pos: dynamic) -> dynamic
  {
      var res: dynamic = 0;
      {
        while ((pos > 0))
        {
          res += arr[pos];
          pos -= lsb(pos);
        }
      }
      return res;
    }
  var arr: dynamic = cpp_uninitialized();
  var sz: dynamic = cpp_uninitialized();
}

var ft: dynamic = cpp_uninitialized();

func fastPow(n: dynamic, p: dynamic) -> dynamic
{
  if ((!p))
  {
    return 1;
  }
  if ((p % 2))
  {
    return ((((1 * n) * fastPow(n, (p - 1)))) % kMod);
  }
  var tmp: dynamic = fastPow(n, (p / 2));
  return ((((1 * tmp) * tmp)) % kMod);
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  read(n);
  ft = cpp_new(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(p[i]);
      if ((p[i] == -1))
      {
        emptyLeft[i] += 1;
        emptyRight[i] += 1;
        i += 1;
        continue;
      }
      seen[p[i]] = true;
      i += 1;
    }
  }
  var cntUnknown: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((!seen[i]))
      {
        cntUnknown += 1;
        countBigger[i] += 1;
        countSmaller[i] += 1;
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      countSmaller[i] += countSmaller[(i - 1)];
      emptyLeft[i] += emptyLeft[(i - 1)];
      i += 1;
    }
  }
  {
    var i: dynamic = n;
    while (i)
    {
      countBigger[i] += countBigger[(i + 1)];
      emptyRight[i] += emptyRight[(i + 1)];
      i -= 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((p[i] != -1))
      {
        res = ((((res + ft->query(n)) - ft->query(p[i]))) % kMod);
        ft->update(p[i]);
        var exp: dynamic = ((((1 * countSmaller[p[i]]) * emptyRight[i])) % kMod);
        exp = ((((1 * exp) * fastPow(cntUnknown, (kMod - 2)))) % kMod);
        res = (((res + exp)) % kMod);
        exp = ((((1 * countBigger[p[i]]) * emptyLeft[i])) % kMod);
        exp = ((((1 * exp) * fastPow(cntUnknown, (kMod - 2)))) % kMod);
        res = (((res + exp)) % kMod);
      }
      i += 1;
    }
  }
  var exp: dynamic = ((((1 * cntUnknown) * ((cntUnknown - 1)))) % kMod);
  exp = ((((1 * exp) * fastPow(4, (kMod - 2)))) % kMod);
  res = (((res + exp)) % kMod);
  write(res);
}
