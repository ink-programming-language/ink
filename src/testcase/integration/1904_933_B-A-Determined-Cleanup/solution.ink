// Translated from solution.cpp.

func operator_shift_left(os: dynamic, v: dynamic) -> dynamic
{
  (os << cpp_char("{"));
  var sep: dynamic = cpp_uninitialized();
  for (var x: dynamic in v)
  {
    ((os << sep) << x);
    sep = ", ";
  }
  return (os << cpp_char("}"));
}

func operator_shift_left(os: dynamic, p: dynamic) -> dynamic
{
  return (((((os << cpp_char("(")) << p.first) << ", ") << p.second) << cpp_char(")"));
}

func dbg_out() -> dynamic
{
  write("\n");
}

func dbg_out(H: dynamic, T: dynamic...) -> dynamic
{
  write(cpp_char(" "), H);
  dbg_out(cpp_expand(T));
}

func output_vector(v: dynamic, add_one: dynamic = false, start: dynamic = -1, end: dynamic = -1) -> dynamic
{
  if ((start < 0))
  {
    start = 0;
  }
  if ((end < 0))
  {
    end = v.size();
  }
  {
    var i: dynamic = start;
    while ((i < end))
    {
      write((v[i] + ( (add_one) ? 1 : 0)), ( ((i < (end - 1))) ? cpp_char(" ") : cpp_char("\n")));
      i += 1;
    }
  }
}

func main() -> dynamic
{
  var P: dynamic = cpp_uninitialized();
  var K: dynamic = cpp_uninitialized();
  read(P, K);
  var A: dynamic = cpp_uninitialized();
  while (true)
  {
    var remainder: dynamic = ((((P % K) + K)) % K);
    A.push_back(remainder);
    P -= remainder;
    P /= (-K);
    if (!(((P != 0))))
    {
      break;
    }
  }
  write(A.size(), cpp_char("\n"));
  output_vector(A);
}
