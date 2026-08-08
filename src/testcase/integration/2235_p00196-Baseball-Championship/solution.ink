// Translated from solution.cpp.

var i: dynamic;

var j: dynamic;

var make = cpp_array(100);

var kati = cpp_array(100);

var name = cpp_array(100);

func change()
{
  var tmp1: dynamic;
  var tmp2: dynamic;
  var tmp3: dynamic;
  tmp1 = kati[(j + 1)];
  kati[(j + 1)] = kati[j];
  kati[j] = tmp1;
  tmp2 = make[(j + 1)];
  make[(j + 1)] = make[j];
  make[j] = tmp2;
  tmp3 = name[(j + 1)];
  name[(j + 1)] = name[j];
  name[j] = tmp3;
}

func main()
{
  var n: dynamic;
  var s: dynamic;
  while (1)
  {
    read(n);
    if ((n == 0))
    {
      break;
    }
    {
      i = 0;
      while ((i < 100))
      {
        kati[i] = 0;
        make[i] = 0;
        i += 1;
      }
    }
    {
      i = 0;
      while ((i < n))
      {
        read(name[i]);
        {
          j = 0;
          while ((j < (n - 1)))
          {
            read(s);
            if ((s == 0))
            {
              kati[i] += 1;
            }
            if ((s == 1))
            {
              make[i] += 1;
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    {
      i = (n - 1);
      while ((i > 0))
      {
        {
          j = 0;
          while ((j < i))
          {
            if ((make[j] > make[(j + 1)]))
            {
              change();
            }
            j += 1;
          }
        }
        i -= 1;
      }
    }
    {
      i = (n - 1);
      while ((i > 0))
      {
        {
          j = 0;
          while ((j < i))
          {
            if ((kati[j] < kati[(j + 1)]))
            {
              change();
            }
            j += 1;
          }
        }
        i -= 1;
      }
    }
    {
      i = 0;
      while ((i < n))
      {
        write(name[i], "\n");
        i += 1;
      }
    }
  }
  return 0;
}
