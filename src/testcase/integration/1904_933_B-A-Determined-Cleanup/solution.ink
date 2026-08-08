// Translated from solution.cpp.

func operator_shift_left(os: dynamic, v: dynamic)
{
  (os << cpp_char("{"));
  var sep: dynamic;
  for (var x in v)
  {
    ((os << sep) << x);
    sep = ", ";
  }
  return (os << cpp_char("}"));
}

func operator_shift_left(os: dynamic, p: dynamic)
{
  return (((((os << cpp_char("(")) << p.first) << ", ") << p.second) << cpp_char(")"));
}

func dbg_out()
{
  write("\n");
}

func dbg_out(H: dynamic, T: dynamic...)
{
  write(cpp_char(" "), H);
  dbg_out(cpp_expand(T));
}

func output_vector(v: dynamic, add_one: dynamic = false, start: dynamic = -1, end: dynamic = -1)
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
    var i = start;
    while ((i < end))
    {
      write((v[i] + (if (add_one) 1 else 0)), (if ((i < (end - 1))) cpp_char(" ") else cpp_char("\n")));
      i += 1;
    }
  }
}

func main()
{
  var P: dynamic;
  var K: dynamic;
  read(P, K);
  var A: dynamic;
  while (true)
  {
    var remainder = ((((P % K) + K)) % K);
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
