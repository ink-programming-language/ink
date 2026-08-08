// Translated from solution.cpp.

func main()
{
  var t = cpp_construct(0);
  var mas = cpp_array(3);
  {
    var i = 0;
    while ((i < 3))
    {
      read(mas[i]);
      i += 1;
    }
  }
  sort(mas, (mas + 3));
  if ((((mas[0] + mas[1])) <= (mas[2] / 2)))
  {
    t = (mas[0] + mas[1]);
  } else
  {
    t = ((((mas[1] + mas[2]) + mas[0])) / 3);
  }
  write(t);
  return 0;
}
