// Translated from solution.cpp.

func main() -> dynamic
{
  var num: dynamic = cpp_uninitialized();
  var temp1: dynamic = cpp_uninitialized();
  var temp2: dynamic = cpp_uninitialized();
  read(num);
  var vi1: dynamic = cpp_uninitialized();
  var vi2: dynamic = cpp_uninitialized();
  var vi3: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while (((i) < (num)))
    {
      read(temp1, temp2);
      vi1.push_back(temp1);
      vi2.push_back(temp2);
      vi3.push_back(temp1);
      vi3.push_back(temp2);
      i += 1;
    }
  }
  sort((vi3).begin(), (vi3).end());
  var lim: dynamic = vi3[(num - 1)];
  {
    var i: dynamic = 0;
    while (((i) < (num)))
    {
      if ((i < (num / 2)))
      {
        write(1);
      } else if ((vi1[i] <= lim))
      {
        write(1);
      } else
      {
        write(0);
      }
      i += 1;
    }
  }
  write("\n");
  {
    var i: dynamic = 0;
    while (((i) < (num)))
    {
      if ((i < (num / 2)))
      {
        write(1);
      } else if ((vi2[i] <= lim))
      {
        write(1);
      } else
      {
        write(0);
      }
      i += 1;
    }
  }
  write("\n");
  return 0;
}
