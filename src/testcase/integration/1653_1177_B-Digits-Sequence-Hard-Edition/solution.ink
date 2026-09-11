// Translated from solution.cpp.

class Debugger
{
  func Debugger(separator: dynamic = ", ") -> dynamic
  {
      self->first = cpp_construct(true);
      self->separator = cpp_construct(separator);
    }
  func operator(v: dynamic) -> dynamic
  {
      if ((!first))
      {
        write(separator);
      }
      write(v);
      first = false;
      return (*self);
    }
  func cpp_destruct_Debugger() -> dynamic
  {
      write("\n");
    }
  var first: dynamic = cpp_uninitialized();
  var separator: dynamic = cpp_uninitialized();
}

func operator_shift_left(os: dynamic, p: dynamic) -> dynamic
{
  return (((((os << "(") << p.first) << ", ") << p.second) << ")");
}

func operator_shift_left(os: dynamic, v: dynamic) -> dynamic
{
  var first: dynamic = true;
  (os << "[");
  {
    var i: dynamic = 0;
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

func operator_shift_left(os: dynamic, v: dynamic) -> dynamic
{
  var first: dynamic = true;
  (os << "[");
  {
    var ii: dynamic = v.begin();
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

func operator_shift_left(os: dynamic, v: dynamic) -> dynamic
{
  var first: dynamic = true;
  (os << "[");
  {
    var ii: dynamic = v.begin();
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

var n: dynamic = cpp_uninitialized();

var ans: dynamic = "";

func kol(n: dynamic) -> dynamic
{
  var ans: dynamic = 9;
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      ans *= 10;
      i += 1;
    }
  }
  return ans;
}

func poww(a: dynamic, b: dynamic) -> dynamic
{
  var res: dynamic = 1;
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

var nuj: dynamic = 0;

var sig: dynamic = 0;

func fff(n: dynamic) -> dynamic
{
  var ans: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      ans += (kol(i) * i);
      i += 1;
    }
  }
  return ans;
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  read(n);
  var raz: dynamic = 1;
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
  var nc: dynamic = (poww(10, (raz - 1)) - 1);
  var nac: dynamic = poww(10, (raz - 1));
  var konec: dynamic = 0;
  var ciss: dynamic = raz;
  while (cpp_update(ciss, "--"))
  {
    konec = ((konec * 10) + 9);
  }
  var minn: dynamic = LLONG_MAX;
  while ((nac <= konec))
  {
    var mid: dynamic = (((nac + konec)) / 2);
    var kol: dynamic = ((((mid - nc)) * raz) + nuj);
    if ((kol >= n))
    {
      minn = min(mid, minn);
      konec = (mid - 1);
    } else
    {
      nac = (mid + 1);
    }
  }
  var nujj: dynamic = to_string(minn);
  var ost: dynamic = (n - ((nuj + (((((minn - 1)) - nc)) * raz))));
  write(nujj[(ost - 1)], "\n");
  return 0;
}
