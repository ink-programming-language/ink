// Translated from solution.cpp.

class Debugger
{
  func Debugger(separator: dynamic = ", ")
  {
      this->first = cpp_construct(true);
      this->separator = cpp_construct(separator);
    }
  func operator(v: dynamic)
  {
      if ((!first))
      {
        write(separator);
      }
      write(v);
      first = false;
      return (*this);
    }
  func ~Debugger()
  {
      write("\n");
    }
  var first: dynamic;
  var separator: dynamic;
}

func operator_shift_left(os: dynamic, p: dynamic)
{
  return (((((os << "(") << p.first) << ", ") << p.second) << ")");
}

func operator_shift_left(os: dynamic, v: dynamic)
{
  var first = true;
  (os << "[");
  {
    var i = 0;
    while ((i < v.size()))
    {
      if ((!first))
      {
        (os << ", ");
      }
      (os << v[i]);
      first = false;
      i += 1;
    }
  }
  return (os << "]");
}

func operator_shift_left(os: dynamic, v: dynamic)
{
  var first = true;
  (os << "[");
  {
    var ii = v.begin();
    while ((ii != v.end()))
    {
      if ((!first))
      {
        (os << ", ");
      }
      (os << (*ii));
      first = false;
      ii += 1;
    }
  }
  return (os << "]");
}

func operator_shift_left(os: dynamic, v: dynamic)
{
  var first = true;
  (os << "[");
  {
    var ii = v.begin();
    while ((ii != v.end()))
    {
      if ((!first))
      {
        (os << ", ");
      }
      (os << (*ii));
      first = false;
      ii += 1;
    }
  }
  return (os << "]");
}

var n: dynamic;

var ans = "";

func kol(n: dynamic)
{
  var ans = 9;
  {
    var i = 1;
    while ((i < n))
    {
      ans *= 10;
      i += 1;
    }
  }
  return ans;
}

func poww(a: dynamic, b: dynamic)
{
  var res = 1;
  {
    while (b)
    {
      if ((b & 1))
      {
        res = ((res * a));
      }
      a *= a;
      b >>= 1;
    }
  }
  return res;
}

var nuj = 0;

var sig = 0;

func fff(n: dynamic)
{
  var ans = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      ans += (kol(i) * i);
      i += 1;
    }
  }
  return ans;
}

func main()
{
  ios_base.sync_with_stdio(0);
  read(n);
  var raz = 1;
  while ((nuj < n))
  {
    nuj += (kol(raz) * raz);
    raz += 1;
  }
  while (1)
  {
    if ((nuj < n))
    {
      break;
    }
    raz -= 1;
    nuj -= (kol(raz) * raz);
  }
  var nc = (poww(10, (raz - 1)) - 1);
  var nac = poww(10, (raz - 1));
  var konec = 0;
  var ciss = raz;
  while (cpp_update(ciss, "--"))
  {
    konec = ((konec * 10) + 9);
  }
  var minn = LLONG_MAX;
  while ((nac <= konec))
  {
    var mid = (((nac + konec)) / 2);
    var kol = ((((mid - nc)) * raz) + nuj);
    if ((kol >= n))
    {
      minn = min(mid, minn);
      konec = (mid - 1);
    } else
    {
      nac = (mid + 1);
    }
  }
  var nujj = to_string(minn);
  var ost = (n - ((nuj + (((((minn - 1)) - nc)) * raz))));
  write(nujj[(ost - 1)], "\n");
  return 0;
}
