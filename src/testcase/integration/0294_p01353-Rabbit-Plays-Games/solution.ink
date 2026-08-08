// Translated from solution.cpp.

class Data
{
  var h: dynamic;
  var a: dynamic;
  var d: dynamic;
  var s: dynamic;
  func Data()
  {
    }
  func Data(h: dynamic, a: dynamic, d: dynamic, s: dynamic)
  {
      this->h = cpp_construct();
      this->a = cpp_construct();
      this->d = cpp_construct();
      this->s = cpp_construct();
    }
  func operator_less(d: dynamic)
  {
      return (s < d.s);
    }
}

func operator_shift_right(is: dynamic, d: dynamic)
{
  return ((((is >> d.h) >> d.a) >> d.d) >> d.s);
}

func solve(M: dynamic, ene: dynamic)
{
  var v: dynamic;
  for (var e in ene)
  {
    if (((M.a <= e.d) && (M.d < e.a)))
    {
      return -1;
    }
    var turn = ceil((cpp_cast(e.h) / ((M.a - e.d))));
    if (((e.a - M.d) <= 0))
    {
      continue;
    }
    v.emplace_back((cpp_cast(turn) / ((e.a - M.d))), e);
  }
  sort(v.begin(), v.end());
  var res = 0;
  var total_turn = 0;
  for (var d in v)
  {
    var e = d.second;
    var turn = ceil((cpp_cast(e.h) / ((M.a - e.d))));
    total_turn += turn;
    var damage = (((total_turn - ((M.s > e.s)))) * max(0, (e.a - M.d)));
    M.h -= damage;
    if ((M.h <= 0))
    {
      return -1;
    }
    res += damage;
  }
  return res;
}

func main()
{
  var N: dynamic;
  read(N);
  var M: dynamic;
  read(M);
  {
    var i = 0;
    while ((i < N))
    {
      read(ene[i]);
      i += 1;
    }
  }
  write(solve(M, ene), "\n");
  return 0;
}
