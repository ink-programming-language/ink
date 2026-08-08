// Translated from solution.cpp.

class vertix
{
  var capacity: dynamic;
  var arr: dynamic;
  func vertix()
  {
      capacity = 000;
      cpp_delete(arr);
      arr = cpp_new();
    }
  func vertix(S: dynamic)
  {
      capacity = S;
      cpp_delete(arr);
      arr = cpp_new();
    }
  func operator_index(index: dynamic)
  {
      return arr[index];
    }
  func size()
  {
      return capacity;
    }
  func push_back(type_cpp: dynamic)
  {
      var newarr = cpp_new();
      {
        var i = 0;
        while ((i < capacity))
        {
          newarr[i] = arr[i];
          i += 1;
        }
      }
      newarr[capacity] = type_cpp;
      cpp_delete(arr);
      arr = newarr;
      capacity += 1;
    }
  func push_front(type_cpp: dynamic)
  {
      var newarr = cpp_new();
      {
        var i = 1;
        while ((i < (capacity + 1)))
        {
          newarr[i] = arr[(i - 1)];
          i += 1;
        }
      }
      newarr[0] = type_cpp;
      capacity += 1;
      cpp_delete(arr);
      arr = newarr;
    }
  func pop_front()
  {
      var newarr = cpp_new();
      {
        var i = 0;
        while ((i < (capacity - 1)))
        {
          newarr[i] = arr[(i + 1)];
          i += 1;
        }
      }
      cpp_delete(arr);
      capacity -= 1;
      arr = newarr;
    }
  func pop_back()
  {
      capacity -= 1;
    }
  func front()
  {
      return arr[0];
    }
  func back()
  {
      return arr[(capacity - 1)];
    }
  func clear()
  {
      capacity = 0;
      var newarr = cpp_new();
      cpp_delete(arr);
      arr = newarr;
    }
  func begin()
  {
      return (&arr[0]);
    }
  func end()
  {
      return (&arr[capacity]);
    }
}

class AbdElrahmanTarek112
{
  var capacity: dynamic;
  var arr: dynamic;
  func AbdElrahmanTarek112()
  {
      capacity = 0;
      arr = cpp_new();
    }
  func AbdElrahmanTarek112(capacity: dynamic)
  {
      this->capacity = capacity;
      arr = cpp_new();
    }
  func operator_index(index: dynamic)
  {
      return arr[index];
    }
  func operator_index(arr: dynamic)
  {
      this->arr = arr;
    }
  func size()
  {
      return capacity;
    }
  func push_back(type_cpp: dynamic)
  {
      var newarr = cpp_new();
      {
        var i = 0;
        while ((i < capacity))
        {
          newarr[i] = arr[i];
          i += 1;
        }
      }
      newarr[capacity] = type_cpp;
      arr = newarr;
      capacity += 1;
    }
  func push_front(type_cpp: dynamic)
  {
      capacity += 1;
      var newarr = cpp_new();
      {
        var i = 1;
        while ((i < capacity))
        {
          newarr[i] = arr[(i - 1)];
          i += 1;
        }
      }
      newarr[0] = type_cpp;
      arr = cpp_new();
      arr = newarr;
    }
  func empty()
  {
      return if ((capacity == 0)) true else false;
    }
  func pop(index: dynamic)
  {
      var newarr = cpp_new();
      {
        var i = 0;
        while ((i < index))
        {
          newarr[i] = arr[i];
          i += 1;
        }
      }
      {
        var i = index;
        while ((i < (capacity - 1)))
        {
          newarr[i] = arr[(i + 1)];
          i += 1;
        }
      }
      cpp_delete(arr);
      capacity -= 1;
      arr = newarr;
    }
  func push_in(type_cpp: dynamic, index: dynamic)
  {
      var newarr = cpp_new();
      {
        var i = 0;
        while ((i < index))
        {
          newarr[i] = arr[i];
          i += 1;
        }
      }
      {
        var i = (index + 1);
        while ((i < (capacity + 1)))
        {
          if (((i - 1) > (capacity - 1)))
          {
            capacity += 1;
          } else
          {
            newarr[i] = arr[(i - 1)];
          }
          i += 1;
        }
      }
      newarr[index] = type_cpp;
      capacity += 1;
      cpp_delete(arr);
      arr = newarr;
    }
  func pop_front()
  {
      var newarr = cpp_new();
      {
        var i = 0;
        while ((i < (capacity - 1)))
        {
          newarr[i] = arr[(i + 1)];
          i += 1;
        }
      }
      cpp_delete(arr);
      capacity -= 1;
      arr = newarr;
    }
  func pop_back()
  {
      capacity -= 1;
    }
  func front()
  {
      return arr[0];
    }
  func back()
  {
      return arr[(capacity - 1)];
    }
  func clear()
  {
      capacity = 0;
      var newarr = cpp_new();
      cpp_delete(arr);
      arr = newarr;
    }
  func begin()
  {
      return (&arr[0]);
    }
  func end()
  {
      return (&arr[capacity]);
    }
  func print()
  {
      if ((capacity == 0))
      {
        return;
      }
      {
        var i = 0;
        while ((i < capacity))
        {
          write(arr[i], " ");
          i += 1;
        }
      }
    }
  func unique()
  {
      var G: dynamic;
      var newcapacity = 0;
      var newindex = 0;
      {
        var i = 0;
        while ((i < capacity))
        {
          if (((i + 1) == capacity))
          {
            G.push_back(arr[i]);
          } else
          {
            if ((arr[i] == arr[(i + 1)]))
            {
            } else
            {
              G.push_back(arr[i]);
            }
          }
          i += 1;
        }
      }
      var newarr = cpp_new();
      {
        var i = 0;
        while ((i < G.size()))
        {
          newarr[i] = G[i];
          i += 1;
        }
      }
      arr = cpp_new();
      arr = newarr;
      capacity = G.size();
    }
  func like_set()
  {
      var c: dynamic;
      var Old_Size = 0;
      var index = 0;
      var newarr = cpp_new();
      {
        var i = 0;
        while ((i < capacity))
        {
          c.insert(arr[i]);
          if ((c.size() != Old_Size))
          {
            Old_Size += 1;
            newarr[index] = arr[i];
            index += 1;
          }
          i += 1;
        }
      }
      arr = newarr;
      capacity = c.size();
    }
}

class var_cpp
{
  var tr: dynamic = cpp_array(10005);
  func operator_index(thekey: dynamic)
  {
      var index = 0;
      {
        var i = 0;
        while ((i < 10005))
        {
          if ((tr[i].first == thekey))
          {
            index = i;
            break;
          }
          i += 1;
        }
      }
      return tr[index].second;
    }
}

func cin_array_usual(o: dynamic, size: dynamic)
{
  {
    var i = 0;
    while ((i < size))
    {
      read(o[i]);
      i += 1;
    }
  }
}

func print_array_usual(o: dynamic, size: dynamic)
{
  {
    var i = 0;
    while ((i < size))
    {
      write(o[i], " ");
      i += 1;
    }
  }
  write("\n");
}

func print_forward_list(o: dynamic)
{
  {
    var i = o.begin();
    while ((i != o.end()))
    {
      write((*i), " ");
      i += 1;
    }
  }
  write("\n");
}

func print_set(o: dynamic)
{
  {
    var i = o.begin();
    while ((i != o.end()))
    {
      write((*i), " ");
      i += 1;
    }
  }
  write("\n");
}

func print_vector(o: dynamic)
{
  {
    var i = o.begin();
    while ((i != o.end()))
    {
      write((*i), " ");
      i += 1;
    }
  }
  write("\n");
}

func main()
{
  var x: dynamic;
  var y: dynamic;
  var z: dynamic;
  read(x, y, z);
  if (((x == y) && (y == z)))
  {
    write(cpp_char("?"), "\n");
  } else if ((((x != y) && (y != z)) && (x != z)))
  {
    write(cpp_char("?"), "\n");
  } else
  {
    if ((x == y))
    {
      if ((x == "rock"))
      {
        if ((z == "paper"))
        {
          write("S", "\n");
        } else if ((z == "scissors"))
        {
          write("?", "\n");
        }
      } else if ((x == "paper"))
      {
        if ((z == "rock"))
        {
          write("?", "\n");
        } else if ((z == "scissors"))
        {
          write("S", "\n");
        }
      } else if ((x == "scissors"))
      {
        if ((z == "rock"))
        {
          write("S", "\n");
        } else if ((z == "paper"))
        {
          write("?", "\n");
        }
      }
    } else if ((y == z))
    {
      if ((y == "rock"))
      {
        if ((x == "paper"))
        {
          write("F", "\n");
        } else if ((x == "scissors"))
        {
          write("?", "\n");
        }
      } else if ((y == "paper"))
      {
        if ((x == "rock"))
        {
          write("?", "\n");
        } else if ((x == "scissors"))
        {
          write("F", "\n");
        }
      } else if ((y == "scissors"))
      {
        if ((x == "rock"))
        {
          write("F", "\n");
        } else if ((x == "paper"))
        {
          write("?", "\n");
        }
      }
    } else if ((x == z))
    {
      if ((x == "rock"))
      {
        if ((y == "paper"))
        {
          write("M", "\n");
        } else if ((y == "scissors"))
        {
          write("?", "\n");
        }
      } else if ((x == "paper"))
      {
        if ((y == "rock"))
        {
          write("?", "\n");
        } else if ((y == "scissors"))
        {
          write("M", "\n");
        }
      } else if ((x == "scissors"))
      {
        if ((y == "rock"))
        {
          write("M", "\n");
        } else if ((y == "paper"))
        {
          write("?", "\n");
        }
      }
    }
  }
  return 0;
}
