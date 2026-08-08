// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var bn: dynamic;
  read(n, bn);
  var sum = 0;
  while (cpp_update(n, "--"))
  {
    var a: dynamic;
    read(a);
    sum = ((cpp_cast(bn) * sum) + a);
  }
  var sum1 = 0;
  var m: dynamic;
  var bm: dynamic;
  read(m, bm);
  while (cpp_update(m, "--"))
  {
    var a: dynamic;
    read(a);
    sum1 = ((cpp_cast(bm) * sum1) + a);
  }
  if ((sum > sum1))
  {
    write(cpp_char(">"));
  } else if ((sum < sum1))
  {
    write(cpp_char("<"));
  } else
  {
    write(cpp_char("="));
  }
}
