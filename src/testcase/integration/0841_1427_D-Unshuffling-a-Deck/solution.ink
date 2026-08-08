// Translated from solution.cpp.

func operator_shift_left(out: dynamic, v: dynamic)
{
  (out << "[");
  for (var k in v)
  {
    ((out << k) << " ");
  }
  ((out << "]") << "\n");
  return out;
}

func operator_shift_left(out: dynamic, s: dynamic)
{
  (out << "{");
  for (var k in s)
  {
    ((out << k) << " ");
  }
  ((out << "}") << "\n");
  return out;
}

func operator_shift_left(out: dynamic, p: dynamic)
{
  (((((out << "[ ") << p.first) << " , ") << p.second) << " ] ");
  return out;
}

func operator_shift_right(in_cpp: dynamic, p: dynamic)
{
  ((in_cpp >> p.first) >> p.second);
  return in_cpp;
}

var res: dynamic;

var n: dynamic;

func Ejecutar(par: dynamic, d: dynamic)
{
  var pars: dynamic;
  var loc: dynamic;
  var p = 0;
  for (var k in par)
  {
    {
      var j = 0;
      while ((j < int_cpp(k)))
      {
        loc.push_back(d[p]);
        p += 1;
        j += 1;
      }
    }
    pars.push_back(loc);
    loc.resize(0);
  }
  reverse(pars.begin(), pars.end());
  var ld: dynamic;
  for (var k in pars)
  {
    for (var j in k)
    {
      ld.push_back(j);
    }
  }
  d = ld;
}

func Paso(d: dynamic, i: dynamic)
{
  if ((d[0] == 1))
  {
    var p = 0;
    while ((d[p] != i))
    {
      p += 1;
    }
    if (((p + 1) != i))
    {
      var par: dynamic;
      {
        var j = 0;
        while ((j < int_cpp((i - 1))))
        {
          par.push_back(1);
          j += 1;
        }
      }
      par.push_back(((p - i) + 2));
      if (((n - p) - 1))
      {
        par.push_back(((n - p) - 1));
      }
      if ((par.size() > 1))
      {
        res.push_back(par);
      }
      Ejecutar(par, d);
      return;
    }
  } else
  {
    var p = 0;
    while ((d[((n - p) - 1)] != i))
    {
      p += 1;
    }
    if (((p + 1) != i))
    {
      var par: dynamic;
      {
        var j = 0;
        while ((j < int_cpp((i - 1))))
        {
          par.push_back(1);
          j += 1;
        }
      }
      par.push_back(((p - i) + 2));
      if (((n - p) - 1))
      {
        par.push_back(((n - p) - 1));
      }
      reverse(par.begin(), par.end());
      if ((par.size() > 1))
      {
        res.push_back(par);
      }
      Ejecutar(par, d);
      return;
    }
  }
}

func main()
{
  cin.tie(0);
  cin.sync_with_stdio(0);
  read(n);
  {
    var i = 0;
    while ((i < int_cpp(n)))
    {
      read(d[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < int_cpp(n)))
    {
      if ((d[i] == 1))
      {
        if (((i != 0) && (i != (n - 1))))
        {
          var p = [i, (n - i)];
          if ((p.size() > 1))
          {
            res.push_back(p);
          }
          Ejecutar(p, d);
        }
        break;
      }
      i += 1;
    }
  }
  {
    var i = int_cpp(2);
    while ((i < int_cpp(n)))
    {
      Paso(d, i);
      i += 1;
    }
  }
  if ((d[0] == n))
  {
    {
      var i = 0;
      while ((i < int_cpp(n)))
      {
        r[i] = 1;
        i += 1;
      }
    }
    if ((r.size() > 1))
    {
      res.push_back(r);
    }
    Ejecutar(r, d);
  }
  write(res.size(), "\n");
  for (var r in res)
  {
    write(r.size(), " ");
    for (var k in r)
    {
      write(k, " ");
    }
    write("\n");
  }
  return 0;
}
