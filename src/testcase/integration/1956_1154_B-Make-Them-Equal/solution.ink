// Translated from solution.cpp.

func main()
{
  var size: dynamic;
  read(size);
  var i: dynamic;
  var arr: dynamic;
  var new_size = 0;
  {
    i = 0;
    while ((i < size))
    {
      var x: dynamic;
      read(x);
      var has = false;
      {
        var j = 0;
        while ((j < i))
        {
          if ((x == arr[j]))
          {
            has = true;
            break;
          }
          j += 1;
        }
      }
      if (has)
      {
        i += 1;
        continue;
      }
      arr.push_back(x);
      new_size += 1;
      i += 1;
    }
  }
  sort(arr.begin(), arr.end());
  if ((new_size > 3))
  {
    write(-1, cpp_char("\n"));
  } else if ((new_size == 3))
  {
    if ((((arr[1] - arr[0])) == ((arr[2] - arr[1]))))
    {
      write((arr[1] - arr[0]), cpp_char("\n"));
    } else
    {
      write(-1, cpp_char("\n"));
    }
  } else if ((new_size == 2))
  {
    if (((((arr[1] - arr[0])) % 2) == 0))
    {
      write((((arr[1] - arr[0])) / 2), cpp_char("\n"));
    } else
    {
      write(((arr[1] - arr[0])), cpp_char("\n"));
    }
  } else if ((new_size == 1))
  {
    write(0, cpp_char("\n"));
  }
}
