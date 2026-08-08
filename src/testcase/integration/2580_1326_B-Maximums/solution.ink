// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var input: dynamic;
  var a1: dynamic;
  var temp: dynamic;
  read(n);
  var a: dynamic;
  var b: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      read(input);
      if ((i == 0))
      {
        b.push_back(input);
        a1 = b[i];
        temp = b[i];
        a.push_back(temp);
        write(a[i], " ");
      } else
      {
        b.push_back(input);
        a1 = (b[i] + temp);
        a.push_back(a1);
        if ((temp < a.back()))
        {
          temp = a.back();
        }
        write(a[i], " ");
      }
      i += 1;
    }
  }
  return 0;
}
