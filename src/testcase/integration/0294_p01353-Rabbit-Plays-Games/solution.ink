// Translated from solution.cpp.

class Data
{
  var h: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  func Data() -> dynamic
  {
    }
  func Data(h: dynamic, a: dynamic, d: dynamic, s: dynamic) -> dynamic
  {
      self->h = cpp_construct();
      self->a = cpp_construct();
      self->d = cpp_construct();
      self->s = cpp_construct();
    }
  func operator_less(d: dynamic) -> dynamic
  {
      return (s < d.s);
    }
}

func operator_shift_right(is: dynamic, d: dynamic) -> dynamic
{
  return ((((is >> d.h) >> d.a) >> d.d) >> d.s);
}

func solve(M: dynamic, ene: dynamic) -> dynamic
{
  var v: dynamic = cpp_uninitialized();
  for (var e: dynamic in ene)
  {
    if (((M.a <= e.d) && (M.d < e.a)))
    {
      return -1;
    }
    var turn: dynamic = ceil((cpp_cast(e.h) / ((M.a - e.d))));
    if (((e.a - M.d) <= 0))
    {
      continue;
    }
    v.emplace_back((cpp_cast(turn) / ((e.a - M.d))), e);
  }
  sort(v.begin(), v.end());
  var res: dynamic = 0;
  var total_turn: dynamic = 0;
  for (var d: dynamic in v)
  {
    var e: dynamic = d.second;
    var turn: dynamic = ceil((cpp_cast(e.h) / ((M.a - e.d))));
    total_turn += turn;
    var damage: dynamic = (((total_turn - ((M.s > e.s)))) * max(0, (e.a - M.d)));
    M.h -= damage;
    if ((M.h <= 0))
    {
      return -1;
    }
    res += damage;
  }
  return res;
}

func main() -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  read(N);
  var M: dynamic = cpp_uninitialized();
  read(M);
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      read(ene[i]);
      i += 1;
    }
  }
  write(solve(M, ene), "\n");
  return 0;
}
