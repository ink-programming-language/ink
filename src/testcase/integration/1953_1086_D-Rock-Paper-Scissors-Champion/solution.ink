// Translated from solution.cpp.

func mod(n: dynamic, m: dynamic) -> dynamic
{
  var ret: dynamic = (n % m);
  if ((ret < 0))
  {
    ret += m;
  }
  return ret;
}

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  return ( ((b == 0)) ? a : gcd(b, (a % b)));
}

func exp(a: dynamic, b: dynamic, m: dynamic) -> dynamic
{
  if ((b == 0))
  {
    return 1;
  }
  if ((b == 1))
  {
    return mod(a, m);
  }
  var k: dynamic = mod(exp(a, (b / 2), m), m);
  if ((b & 1))
  {
    return mod((a * mod((k * k), m)), m);
  } else
  {
    return mod((k * k), m);
  }
}

class Bit
{
  var BIT: dynamic = cpp_uninitialized();
  var N: dynamic = cpp_uninitialized();
  func Bit() -> dynamic
  {
    }
  func Bit(n: dynamic) -> dynamic
  {
      BIT.resize((n + 100), 0);
      N = (n + 10);
    }
  func update(x: dynamic, v: dynamic) -> dynamic
  {
      while ((x < N))
      {
        BIT[x] += v;
        x += ((x & (-x)));
      }
    }
  func sum(x: dynamic) -> dynamic
  {
      var r: dynamic = 0;
      while ((x > 0))
      {
        r += BIT[x];
        x -= ((x & (-x)));
      }
      return r;
    }
  func query(l: dynamic, r: dynamic) -> dynamic
  {
      return (sum(r) - sum((l - 1)));
    }
}

var mat: dynamic = cpp_array(200100);

var S: dynamic = cpp_array(3);

var BIT: dynamic = cpp_uninitialized();

func upd(pos: dynamic, c: dynamic, f: dynamic) -> dynamic
{
  if (f)
  {
    BIT[c].update(pos, 1);
    S[c].insert(pos);
  } else
  {
    var last: dynamic = mat[pos];
    BIT[last].update(pos, -1);
    S[last].erase(pos);
    mat[pos] = c;
    BIT[c].update(pos, 1);
    S[c].insert(pos);
  }
}

var n: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

func ask() -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < 3))
    {
      if ((S[i].size() == 0))
      {
        if ((i == 2))
        {
          write(S[1].size(), "\n");
          return;
        }
        if ((i == 0))
        {
          if ((S[2].size() == 0))
          {
            write(S[1].size(), "\n");
          } else
          {
            write(S[2].size(), "\n");
          }
          return;
        }
        if ((i == 1))
        {
          write(S[0].size(), "\n");
          return;
        }
        return;
      }
      i += 1;
    }
  }
  var res: dynamic = n;
  {
    var i: dynamic = 0;
    while ((i < 3))
    {
      if ((i == 0))
      {
        var l: dynamic = (*S[1].begin());
        var r: dynamic = (*S[2].begin());
        if ((l <= r))
        {
          res -= BIT[i].query(l, r);
        }
        r = (*S[1].rbegin());
        l = (*S[2].rbegin());
        if ((l <= r))
        {
          res -= BIT[i].query(l, r);
        }
      }
      if ((i == 1))
      {
        var l: dynamic = (*S[2].begin());
        var r: dynamic = (*S[0].begin());
        if ((l <= r))
        {
          res -= BIT[i].query(l, r);
        }
        r = (*S[2].rbegin());
        l = (*S[0].rbegin());
        if ((l <= r))
        {
          res -= BIT[i].query(l, r);
        }
      }
      if ((i == 2))
      {
        var l: dynamic = (*S[0].begin());
        var r: dynamic = (*S[1].begin());
        if ((l <= r))
        {
          res -= BIT[i].query(l, r);
        }
        r = (*S[0].rbegin());
        l = (*S[1].rbegin());
        if ((l <= r))
        {
          res -= BIT[i].query(l, r);
        }
      }
      i += 1;
    }
  }
  write(res, "\n");
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  read(n, q);
  var mp: dynamic = cpp_uninitialized();
  mp[cpp_char("P")] = 0;
  mp[cpp_char("S")] = 1;
  mp[cpp_char("R")] = 2;
  BIT = vector(3);
  {
    var i: dynamic = 0;
    while ((i < 3))
    {
      BIT[i] = Bit(n);
      {
        var j: dynamic = 0;
        while ((j < (n + 10)))
        {
          BIT[i].BIT[j] = 0;
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var cc: dynamic = cpp_uninitialized();
      read(cc);
      mat[i] = mp[cc];
      upd(i, mat[i], 1);
      i += 1;
    }
  }
  ask();
  while (cpp_update(q, "--"))
  {
    var p: dynamic = cpp_uninitialized();
    var cc: dynamic = cpp_uninitialized();
    read(p, cc);
    var c: dynamic = mp[cc];
    upd(p, c, 0);
    ask();
  }
}
